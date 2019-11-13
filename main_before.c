/*
Reference: https://blog.csdn.net/zqixiao_09/article/details/50298693
*/

/* daq_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include <math.h>

/* pt_args.c */
#include <pthread.h>


#define	INTERRUPT			iobase[1] + 0				// Badr1 + 0 : also ADC register
#define	MUXCHAN				iobase[1] + 2				// Badr1 + 2
#define	TRIGGER				iobase[1] + 4				// Badr1 + 4
#define	AUTOCAL				iobase[1] + 6				// Badr1 + 6
#define DA_CTLREG			iobase[1] + 8				// Badr1 + 8

#define AD_DATA				iobase[2] + 0				// Badr2 + 0
#define AD_FIFOCLR			iobase[2] + 2				// Badr2 + 2

#define	TIMER0				iobase[3] + 0				// Badr3 + 0
#define	TIMER1				iobase[3] + 1				// Badr3 + 1
#define	TIMER2				iobase[3] + 2				// Badr3 + 2
#define	COUNTCTL			iobase[3] + 3				// Badr3 + 3
#define	DIO_PORTA			iobase[3] + 4				// Badr3 + 4
#define	DIO_PORTB			iobase[3] + 5				// Badr3 + 5
#define	DIO_PORTC			iobase[3] + 6				// Badr3 + 6
#define	DIO_CTLREG			iobase[3] + 7				// Badr3 + 7
#define	PACER1				iobase[3] + 8				// Badr3 + 8
#define	PACER2				iobase[3] + 9				// Badr3 + 9
#define	PACER3				iobase[3] + a				// Badr3 + a
#define	PACERCTL			iobase[3] + b				// Badr3 + b

#define DA_Data				iobase[4] + 0				// Badr4 + 0
#define DA_FIFOCLR			iobase[4] + 2				// Badr4 + 2

#define	DEBUG						1
#define NUM_THREADS					3

/* Global Variable */
int badr[5];											// PCI 2.2 assigns 6 IO base addresses
uintptr_t iobase[6];
uintptr_t dio_in;
uint16_t adc_in;

struct thread_data
{
	int thread_id;
};

struct waveform_properties{
    int type_wave;                  // 0: Sine Wave     	   1: Square Wave      
    int number_of_points;			// 2: Triangular Wave      3: Sawtooth Wave
    int amplitude;
};

struct thread_data thread_data_array[NUM_THREADS];

/* Function Declaration */
void *ReadSwitchStatus(void *arg);
void *PrintHello(void *arg);
void *Out_wave(void *arg);
int generate_waveform(struct waveform_properties arg)；

int main() {
	struct pci_dev_info info;
	void *hdl;

	unsigned int i,count;
	unsigned short chan;

	int t,rc;

	pthread_t threads[NUM_THREADS];
	pthread_attr_t pthread_attr;
	pthread_attr_init(&pthread_attr);
	pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);

	memset(&info,0,sizeof(info));				// memset() is used to fill a block of memory with a particular value.
						 						// https://www.geeksforgeeks.org/memset-c-example/

	if(pci_attach(0)<0) {									// Connect to the PCI server
		perror("pci_attach");
		exit(EXIT_FAILURE);
		}

	/* Vendor and Device ID */				 	// Datatype: struct pci_dev_info
	info.VendorId=0x1307;
	info.DeviceId=0x01;

	// Attach a driver to a PCI device
	if ((hdl=pci_attach_device(0, PCI_SHARE|PCI_INIT_ALL, 0, &info))==0) {
		perror("pci_attach_device");
		exit(EXIT_FAILURE);
		}

	for(i=0;i<6;i++) {
		if(info.BaseAddressSize[i]>0){
			printf("Aperture %d  Base 0x%x Length %d Type %s\n", i,
				PCI_IS_MEM(info.CpuBaseAddress[i]) ?  (int)PCI_MEM_ADDR(info.CpuBaseAddress[i]) :
				(int)PCI_IO_ADDR(info.CpuBaseAddress[i]),info.BaseAddressSize[i],
				PCI_IS_MEM(info.CpuBaseAddress[i]) ? "MEM" : "IO");
		}// Test whether the address is a memory address.
	}

	printf("IRQ %d\n",info.Irq); 				// IRQ: Interrupt number (output)

	// Assign BADRn IO addresses for PCI-DAS1602
	if(DEBUG) {
		printf("\nDAS 1602 Base addresses:\n\n");
		for(i=0;i<5;i++){
		/* PCI_IO_ADDR: Convert the address returned by the PCI server to an I/O address. */
			badr[i]=PCI_IO_ADDR(info.CpuBaseAddress[i]);
			if(DEBUG) printf("Badr[%d] : %x\n", i, badr[i]);
	}

		printf("\nReconfirm Iobase:\n");  			// map I/O base address to user space

	for(i=0;i<5;i++) {								// expect CpuBaseAddress to be the same as iobase for PC
		/* mmap_device_io(size_t len, uint64_t io) : Gain access to a device's registers
			len: The number of bytes of device I/O memory that you want to access. It can't be 0.
				io : The address of the area that you want to access.

			Return:
			A handle to the device's I/O memory, or MAP_DEVICE_FAILED if an error occurs
		*/
		iobase[i]=mmap_device_io(0x0f,badr[i]);
		printf("Index %d : Address : %x ", i,badr[i]);
		printf("IOBASE  : %x \n",iobase[i]);
		}
	}

	// Modify thread control privity
	if(ThreadCtl(_NTO_TCTL_IO,0)==-1) {
		perror("Thread Control");
		exit(1);
		}

	for(t=0;t<NUM_THREADS;t++) {
		thread_data_array[t].thread_id = t;
		printf("Creating thread %d\n", t);

		/*
		Thread No.
		t = 0 : Read Switch Status
		t = 1 : Print Switch Status
		t = 2 : Output waveform
		*/
		if(t==0){
			/* Read Switch Status */
			rc = pthread_create(&threads[t], &pthread_attr, (void *)ReadSwitchStatus, (void *)
						&thread_data_array[t]);
		}
		else if(t==1){
			rc = pthread_create(&threads[t], &pthread_attr, (void *)PrintHello, (void *)
				&thread_data_array[t]);
		}
		else{
			rc = pthread_create(&threads[t], &pthread_attr, (void *)Out_wave, (void *)
				&thread_data_array[t]);
		}

		if (rc) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
			}
		}

		pthread_attr_destroy(&pthread_attr);

		while(1){
		}
}

/* Function */
int generate_waveform(struct waveform_properties arg){
    number_of_points = arg.number_of_points;
    type_wave = arg.type_wave;
    amplitude = arg.amplitude;

    unsigned int i,count;
	float delta,dummy;
	unsigned int data[number_of_points];
	sleep(1.0);

    delta = (2.0*3.142) / number_of_points;					// increment

    if (type_wave == 0) {
        printf("Sine Wave\n");
        for (i = 0; i<number_of_points; i++) {
            dummy= ((sinf((float)(i*delta))) + 1.0) * amplitude ;
            data[i]= (unsigned) dummy;			// add offset +  scale
        }
    }
    else if(type_wave == 1){
        printf("Square Wave\n");
        for (i = 0; i<number_of_points; i++) {
            if(i<25){
                data[i]= 2 * amplitude -1;
            }
            else{
                data[i]=0;
            }
    }
}
    else if(type_wave == 2){
        printf("Triangular Wave\n");
        for(i=0;i<int(number_of_points/4);i++){
            dummy = (i*delta)*amplitude;
            data[i]= (unsigned) dummy;
        }
        for(i=int(number_of_points/4);i<int(3*number_of_points/4);i++){
            dummy = data[i-1] - delta*amplitude;
            data[i]= (unsigned) dummy;
        }
        for(i=int(3*number_of_points/4);i<int(number_of_points);i++){
            dummy = data[i-1] + delta*amplitude;
            data[i]= (unsigned) dummy;
        }
}
    else{
        printf("Sawtooth Wave\n");
        for(i=0;i<int(number_of_points/4);i++){
            dummy = (i*delta)*amplitude;
            data[i]= (unsigned) dummy;
        }
        float gap_1 = data[number_of_points/4-1];
        for(i=int(number_of_points/4);i<int(3*number_of_points/4);i++){
            ummy = -amplitude*gap_1+ (i*delta)*amplitude ;
            data[i]= (unsigned) dummy;
        }
        float gap_2 = -amplitude*gap_1+ ((3*interval/4)*delta)*amplitude;
        for(i=int(3*number_of_points/4);i<int(number_of_points);i++){
            dummy = -amplitude*gap_1+ (i*delta)*amplitude - gap_2;
            data[i]= (unsigned) dummy;
        }
    }

    return data;
}
void *PrintHello(void *arg)
{
	int taskid_2;
	struct thread_data *my_data_2;
	sleep(1);
	my_data_2 = (struct thread_data *) arg;
	taskid_2 = my_data_2->thread_id;
	while(1){
	//printf("Thread %d -> Port A : %x\n", taskid_2, dio_in-0xF0);
	}
}

void *ReadSwitchStatus(void *arg){
	int taskid_1;
  struct thread_data *my_data_1;
  my_data_1 = (struct thread_data *) arg;
  taskid_1 = my_data_1->thread_id;
  while(1){
 //printf("Thread %d (Read Switch Status)\n", taskid_1);

  out8(DIO_CTLREG,0x90);					// Port A : Input,  Port B : Output,
														// Port C (upper | lower) : Output | Output

  dio_in=in8(DIO_PORTA); 					// Read Port A
  //printf("Port A : %02x\n", dio_in);	// Beh: <- switch status

  out8(DIO_PORTB, dio_in);					// output Port A value -> write to Port B
  	sleep(1);
  }

}

void *Out_wave(void *arg) {
	//**********************************************************************************************
	// Setup waveform array
	//**********************************************************************************************

	unsigned int i,count;
	float delta,dummy;
	unsigned int data[50];
	sleep(1.0);

	while(1){
			delta = (2.0*3.142) / 50.0;					// increment

		if (((int) dio_in)-240 == 0) {
			printf("Sine Wave\n");
			for (i = 0; i<50; i++) {
				dummy= ((sinf((float)(i*delta))) + 1.0) * 0x7000 ;
				data[i]= (unsigned) dummy;			// add offset +  scale
			}
		}
		else{
			printf("Square Wave\n");
			for (i = 0; i<50; i++) {
				if(i<25){
					data[i]= 2 * 0x5000 -1;
				}
				else{
					data[i]=0;
				}
		}
	}

	//*********************************************************************************************
	// Output wave
	//*********************************************************************************************

		for (i = 0; i<50; i++) {
			out16(DA_CTLREG, 0x0a23);			// DA Enable, #0, #1, SW 5V unipolar		2/6
			out16(DA_FIFOCLR, 0);					// Clear DA FIFO  buffer
			out16(DA_Data, (short)data[i]);
			out16(DA_CTLREG, 0x0a43);			// DA Enable, #1, #1, SW 5V unipolar		2/6
			out16(DA_FIFOCLR, 0);					// Clear DA FIFO  buffer
			out16(DA_Data, (short)data[i]);
		}
	}
	sleep(1);
}
/* Function */
