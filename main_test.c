// 09September 2005
//******************************************************************************************************
// Performs basic I/O for the Omega PCI-DAS1602
//
// Demonstration routine to demonstrate pci hardware programming
// Demonstrated the most basic DIO and ADC and DAC functions
// - excludes FIFO and strobed operations
//
// Note :
//
//			22 Sept 2016 : Restructured to demonstrate Sine wave to DA
//
// G.Seet - 26 August 2005
//******************************************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <hw/pci.h>
#include <hw/inout.h>
#include <sys/neutrino.h>
#include <sys/mman.h>
#include <math.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>

#define	INTERRUPT		iobase[1] + 0				// Badr1 + 0 : also ADC register
#define	MUXCHAN			iobase[1] + 2				// Badr1 + 2
#define	TRIGGER			iobase[1] + 4				// Badr1 + 4
#define	AUTOCAL			iobase[1] + 6				// Badr1 + 6
#define DA_CTLREG		iobase[1] + 8				// Badr1 + 8

#define	 AD_DATA		iobase[2] + 0				// Badr2 + 0
#define	 AD_FIFOCLR		iobase[2] + 2				// Badr2 + 2

#define	TIMER0			iobase[3] + 0				// Badr3 + 0
#define	TIMER1			iobase[3] + 1				// Badr3 + 1
#define	TIMER2			iobase[3] + 2				// Badr3 + 2
#define	COUNTCTL		iobase[3] + 3				// Badr3 + 3
#define	DIO_PORTA		iobase[3] + 4				// Badr3 + 4
#define	DIO_PORTB		iobase[3] + 5				// Badr3 + 5
#define	DIO_PORTC		iobase[3] + 6				// Badr3 + 6
#define	DIO_CTLREG		iobase[3] + 7				// Badr3 + 7
#define	PACER1			iobase[3] + 8				// Badr3 + 8
#define	PACER2			iobase[3] + 9				// Badr3 + 9
#define	PACER3			iobase[3] + a				// Badr3 + a
#define	PACERCTL		iobase[3] + b				// Badr3 + b

#define DA_Data			iobase[4] + 0				// Badr4 + 0
#define	DA_FIFOCLR		iobase[4] + 2				// Badr4 + 2

int badr[5];													// PCI 2.2 assigns 6 IO base addresses
uintptr_t iobase[6];
uintptr_t dio_in;
uint16_t adc_in;
float freq = 1.0;
unsigned int data[100];         // Bang Yi, why got this line?

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*void setSineArray(float amp, float mean, float points)
{
	float delta = (2.0*3.142)/points;
	int i = 0;
	float dummy;

	for(i = 0; i< (int) points ; i++)
	{
		dummy = ((sinf((float)(i*delta))) + mean) * 0x7fff * amp;
		data[i] = (unsigned) dummy;
	}
}*/

struct waveform_properties
{
    int type_wave;                  // 0: Sine Wave     	   1: Square Wave
    int number_of_points;			// 2: Triangular Wave      3: Sawtooth Wave
    int amplitude;
};

/* Set wave array function */
unsigned int* generate_waveform(struct waveform_properties arg)
{
    int number_of_points = arg.number_of_points;
    int type_wave = arg.type_wave;
    int amplitude = arg.amplitude;


    unsigned int i;
    float delta,dummy;
    float gap_1,gap_2;
    unsigned int *data = malloc(sizeof(unsigned int)*number_of_points);
    sleep(1.0);

    delta = (2.0*3.142) / number_of_points;					// increment

    if (type_wave == 0)
    {
        printf("Sine Wave\n");
        for (i = 0; i<number_of_points; i++)
        {
            dummy= ((sinf((float)(i*delta))) + 1.0) * amplitude ;
            data[i]= (unsigned) dummy;			// add offset +  scale
        }
    }
    else if(type_wave == 1)
    {
        printf("Square Wave\n");
        for (i = 0; i<number_of_points; i++)
        {
            if(i< (int) number_of_points/2)
            {
                data[i]= 2 * amplitude -1;
            }
            else
            {
                data[i]=0;
            }
        }
    }
    else if(type_wave == 2)
    {
        printf("Triangular Wave\n");

        for(i = 0; i < (int) number_of_points/2; i++)
        {
        	//dummy = (i/number_of_points) * amplitude*2;
            dummy = i*delta* amplitude;
        	data[i] = (unsigned) dummy;
        }

        for(i ; i < (int) number_of_points ; i++)
        {
        	//dummy = data[i-1] - (i- (int)number_of_points/2 -1)*delta*amplitude;
        	//dummy = (0.5 - (i - (int)number_of_points/2)/number_of_points  ) * amplitude * 2;
            dummy = delta[i-1] - delta*amplitude;
        	data[i] = (unsigned) dummy;
        }
    }
    else
    {
        printf("Sawtooth Wave\n");
        for(i = 0; i < (int)number_of_points; i ++)
        {
        	dummy =  (i*delta/3.142)*amplitude;
        	data[i] = (unsigned) dummy;
        }
    }

    return data;
}

void OutputWave (struct waveform_properties *wave_prop, float freq, float minTimeBetPoints)
{
    //**********************************************************************************************
    // Setup waveform array
    //**********************************************************************************************

    //struct waveform_properties wave_prop;
    unsigned int* data;
    unsigned int i;
    float delta, dummy;
    struct timespec timeStruct;
    double currentTime = 0, prevTime = 0;
    float points = 0;

    printf("Start output function\n");

    wave_prop->number_of_points = (int) 1.0/freq/minTimeBetPoints;
    //printf("%f\n", freq);

    data = generate_waveform(*wave_prop);

    points = wave_prop->number_of_points;

    //*********************************************************************************************
    // Output wave
    //*********************************************************************************************

    i = 0;
    while(1)
    {
        clock_gettime(CLOCK_REALTIME, &timeStruct);
        currentTime = (double) timeStruct.tv_sec + (double) timeStruct.tv_nsec/1000000000.0;


        if( currentTime - prevTime > minTimeBetPoints/2 && i < (int)points)
        {
            out16(DA_CTLREG,0x0a43);			// DA Enable, #1, #1, SW 5V unipolar		2/6
            out16(DA_FIFOCLR, 0);					// Clear DA FIFO  buffer
            out16(DA_Data,(short) data[i]);

            clock_gettime(CLOCK_REALTIME, &timeStruct);
            prevTime = (double) timeStruct.tv_sec + (double) timeStruct.tv_nsec/1000000000.0;
            i++;
        }

//      printf("%d\n", (int) data[i]);

        if(i == (int)points)
            i = 0;
    }
}


int main()
{
    //struct sched_param param;
    struct pci_dev_info info;
    struct waveform_properties wave_prop;
    void *hdl;

    float minTimeBetPoints = 0;

    unsigned int i,count;
    unsigned short chan;

    //Get minimum time between points
    struct timespec res;
    clock_getres(CLOCK_REALTIME, &res);
    minTimeBetPoints = (double)res.tv_nsec/1000000000.0;

    printf("\fDemonstration Routine for PCI-DAS 1602\n\n");

    memset(&info,0,sizeof(info));
    if(pci_attach(0)<0)
    {
        perror("pci_attach");
        exit(EXIT_FAILURE);
    }

    /* Vendor and Device ID */
    info.VendorId=0x1307;
    info.DeviceId=0x01;

    if ((hdl=pci_attach_device(0, PCI_SHARE|PCI_INIT_ALL, 0, &info))==0)
    {
        perror("pci_attach_device");
        exit(EXIT_FAILURE);
    }

    // Determine assigned BADRn IO addresses for PCI-DAS1602
    printf("\nDAS 1602 Base addresses:\n\n");
    for(i=0; i<5; i++)
    {
        badr[i]=PCI_IO_ADDR(info.CpuBaseAddress[i]);
        printf("Badr[%d] : %x\n", i, badr[i]);
    }

    printf("\nReconfirm Iobase:\n");  						// map I/O base address to user space
    for(i=0; i<5; i++)  												// expect CpuBaseAddress to be the same as iobase for PC
    {
        iobase[i]=mmap_device_io(0x0f,badr[i]);
        printf("Index %d : Address : %x ", i,badr[i]);
        printf("IOBASE  : %x \n",iobase[i]);
    }
    // Modify thread control privity
    if(ThreadCtl(_NTO_TCTL_IO,0)==-1)
    {
        perror("Thread Control");
        exit(1);
    }

    //Define waveform property
    wave_prop.amplitude = 0x7fff;
    wave_prop.type_wave = 2;
    wave_prop.number_of_points = 50;

    //Output wave
    OutputWave(&wave_prop, 1.0, minTimeBetPoints);

    return(0);
}
