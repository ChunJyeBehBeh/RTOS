#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
int get_n();
void check_command_argument(int no_argument,char **argument_address);
float get_mean(float* inputs, int array_size);
float get_std(float* inputs, int array_size, float mean);
void display(float* inputs, int array_size, float mean, float std);
void print_lines(int array_size);
#define min(x,y) ((x) <= (y)) ? (x) : (y)			// find the min number between two number 

int main(int argc, char *argv[] )
{	
    int n, i;
    float *usr_input, mean, std;
	if(argc==1 ){
   		printf("By User input\n");
        n = get_n(); 								//Get size of input for user
	}
    else if(argc>1 ){
   		check_command_argument(argc,&argv[1]);
	}

	if(argc>1) 						 //Check if there are more than 1 argument
	{
		char **p_to_arg = &argv[1];  //Point to argv[1] since argv[0] is program file name (Irrelevant)
		//Check for input method -n=Command line, -a=program input->Run full script
		switch((*p_to_arg)[1])
		{
			case 'n' : printf("By command argument\n"); 	//Check if -n = by command line
					   p_to_arg++;   						// Point to input array of numbers
					   n =atoi(*p_to_arg);					// Convert Character String to Integer [stdlib.h]
					   if(n<=0) 							// Check if number of input size <0
					   {
						printf("You entered a negative value. Please enter a value more than 0.\n");
						exit(1);
					   }
					   else
					   {
					   		if(argc!=n+3)  //Offset 3 for ./program_name -n and size.   (./RTOS -n 3) are the 3 inputs.
							{/*Check if input array size is the same as user defined input size*/
								printf("Didn't input the correct number of data!\n");
								exit(1);
							}
							else
							{
				   				break;
							}
					   	}
			case 'a' : printf("By User input\n");
					   n = get_n(); 						//Get size of input for user
					   break;
			default	 : printf("Invalid option.\n");
					   exit(1);
		}
	}

    if((usr_input=(float*)malloc(n*sizeof(float))) == NULL){ //Memory allocation
        printf("Not enough memory. Use a smaller data size.");
    }
    if(argc>1 && strcmp(argv[1],"-n")==0){
		/*If command line input. Run this script.
		Input all user input array to *usr_input*/
    	int j=0;
        char **p_to_arg = &argv[3];
    	for(j=0;j<n;j++){
			usr_input[j]=atof(*p_to_arg); 					// Convert string to float
			p_to_arg++;
		}
	}
	else {
		/*Script for user input*/
		int value;
	    for(i=0; i < n; i++){
	        while(1){  									  // Get number of inputs from user

	        printf("Enter value of %d/%d: ", (i+1), n);
	        if(scanf("%f", &usr_input[i]) == 1) 		  // Check if input value from reader is a valid float type
	        {
	        	value=getchar();						  // Get the value in buffer after scanf(). 
				if(value !='\n')						  // Check whether the user input a invalid number, 
				{									      // if it is invalid, the value in buffer will not equal to '\n'
					printf("Invalid Input end with alphabet.\n");
					exit(1);
				}
	            fseek(stdin,0,SEEK_END); 				  // Clear io buffer once done
	            break; 									  // Break out of while loop
	        }
	        // Error block to catch for non float value
	        printf("Error. You did not enter number. Please enter a number\n");
	        fseek(stdin,0,SEEK_END);					  // Clear io buffer once done
	        }
	    }
	}
    mean = get_mean(usr_input, n);						  // Calculate the mean
    std = get_std(usr_input, n, mean);					  // Calculate the standard deviation
    display(usr_input, n, mean, std);					  // Display the result
    free(usr_input); 									  //Clear malloc
}
void check_command_argument(int no_argument,char **argument_address)
{
	if(no_argument>1)
	{
	   if(strcmp((*argument_address),"--help")==0)
	   {
			printf("--- Here is the Documentation ---\n");
			printf("This program is to compute the mean and standard deviation of a given sample.\n");
			printf("Run either one of the commands below in terminal.\n");
			printf("(i)  RTOS_CA.exe -n {number of input} {first_number} {second_number} .... {n_number}.\n");
			printf("(ii) RTOS_CA.exe -a user_input.\n");
			exit(1);
		}
		while(--no_argument && (*argument_address)[0]=='-')	// This while loop is to check whether has invalid arguments exist, 
		{													// such as -na -na -abc -3
//			printf("checking argument\n");
			if((*argument_address)[1]=='\0')
			{
				printf("Input - without option. Expected -n/-a argument pass in via command line.\n");
				exit(1);
			}
			else
			{
				if((*argument_address)[1]=='n' || (*argument_address)[1]=='a')
				{
					if((*argument_address)[2]=='\0')
					{
//						printf("Valid option.\n");
						break;
					}
					else{
						printf("Invalid option. Valid Option is either a or n. Please see the documentation by execute command <RTOS_CA.exe --help> in the terminal.\n");
						exit(1);
					}
				}
				else{
					printf("Invalid Option.\n");
					exit(0);
				}
			}
			argument_address++;	
		}
//		printf("Done checking argument\n");

		if(strcmp((*argument_address),"-n")==0)				//** This block is to check whether user give the valid float/int number as samples
		{
			argument_address++;								//** Point to the first data which is argv[1] 
			int int_input;
			float flt_input;
			int counter_1,counter_2=0, interation=no_argument-1;    // -1 because of the first argument ./RTOS
			for(counter_1=0;counter_1<interation;counter_1++)
			{
				if(counter_1 ==0){						
					while(isdigit((*argument_address)[counter_2])){	//** The first number is number of data,
					counter_2 ++;									//** counter_2++ is used to iterate the whole argument to check all is digit number
				}
				}
				if(counter_1>0){									//** Other that the first argument, it should be a float such as 73.84, so ASCII 46 is the character '.' 
				while(isdigit((*argument_address)[counter_2]) || (*argument_address)[counter_2] ==46){
					counter_2 ++;
				}
				}
				if(((*argument_address)[counter_2])=='\0'){			//** After checking all the number, the last check is to check whether the buffer is '\0'
					break;											//** to avoid the input such as 73.84a where the buffer is 'a' instead of '\0'
								}
				else{
					printf("Please enter a Integer or Float Number.\n");
					exit(1);
				}
				counter_2=0;										//** reset the counter to iterate from starting character for the next argument 
				argument_address++;									// Point to the next argument 
			}
		}
	}
}

int get_n()
{
    /*Macro to get size of input from user*/
    int n,value;
    while(1){  //Get number of inputs from user

        printf("Input the number of data points to be computed: ");
        if(scanf("%d", &n) == 1 && n > 0) 		//Check if input value from reader is a valid float type and more than 0
        {
        	value=getchar();
			if(value !='\n')
			{
				printf("Invalid Input end with alphabet.\n");
				exit(1);
			}
            fseek(stdin,0,SEEK_END);
            break; 								//Break out of while loop
        }
        // Error block
        else if (n <= 0)
        {
            printf("You entered a negative value or invalid number. Please enter a value more than 0.\n");  //Check for negative value
            fseek(stdin,0,SEEK_END);
        }
        else{
        printf("Error. You did not enter a whole number. Please enter a whole number value.\n"); //Catch block for other errors: Wrong type of data eg:str
        fseek(stdin,0,SEEK_END);
        }
    }
    return n;
}

float get_mean(float* inputs, int array_size){
    /*Macro to get the mean value.
    ------------------------------
    inputs: float pointer to user input of float[]
    array_size: Int value of the size of array*/
    int i;
    float sum = 0;

    for (i = 0; i < array_size; i++)
    {
        sum += inputs[i];
    }
 return sum/array_size;
}

float get_std(float* inputs, int array_size, float mean){
    /*Macro to get standard deviation
    -----------------------------------
    inputs: float pointer to user input of float[]
    array_size: Int value of the size of array
    mean: float mean of inputs*/
    float std, sum;
    int i;
    for (i = 0; i < array_size; i++)
    {
        sum += pow((inputs[i] - mean), 2);
    }
    return sqrt(sum/array_size);
}

void display(float* inputs, int array_size, float mean, float std){
    /* Macro to format display
    --------------------------
    inputs: float pointer to user input of float[]
    array_size: Int value of the size of array
    mean: float mean of inputs
    std: float of standard deviation of inputs*/
    int i;
    print_lines(array_size);
    printf("Results\n");
    print_lines(array_size);
    printf("Your %d inputs are: ", array_size);
    for (i = 0; i < array_size; i++)
    {
        printf("%.2f, ", inputs[i]);
    }
    printf("\n");
    print_lines(array_size);
    printf("Mean of the inputs is %.3f\n", mean);
    printf("Standard deviation of the inputs is %.3f\n", std);
    print_lines(array_size);
}

void print_lines(int array_size){
    /*Function to print lines in relation to array size for display()
    -----------------------------------------------------------------
    array_size: int of size of user input*/
    int i;
    int a = min(array_size,12);				// choose the minimum number to avoid print too much line
    for (i = 0; i < a; i++)
    {
        printf("------------------");
    }
    printf("\n");
}
