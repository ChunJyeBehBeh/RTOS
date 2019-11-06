/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

Reference: https://onlinegdb.com/HkptZgksS

*******************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define NUM_THREADS 2
int status=0;

struct thread_data
{
	int	thread_id;
};

struct thread_data thread_data_array[NUM_THREADS];

void *Add(void *arg){
    printf("New Thread 0\n");
    int taskid_1;
    struct thread_data *my_data_1;
    
    sleep(1);
    my_data_1 = (struct thread_data *) arg;
    taskid_1 = my_data_1->thread_id;		
    printf("Thread %d ", taskid_1);	
    printf("Status: %d\n",status);
    status = status+1;
}

void *Print(void *arg){
    printf("New Thread 1\n");
    int taskid_2;
    struct thread_data *my_data_2;
    
    sleep(1);
    my_data_2 = (struct thread_data *) arg;
    taskid_2 = my_data_2->thread_id;
    sleep(1);
    printf("Thread %d ", taskid_2);	
    printf("Status: %d\n",status);
}

int main()
{
    int t,rc;
    pthread_t threads[NUM_THREADS];	
    printf("Hello World\n");
    for(t=0;t<NUM_THREADS;t++){
        thread_data_array[t].thread_id = t;
        
         if(t==0){
            /* Read Switch Status */
            rc = pthread_create(&threads[t], NULL, (void *)Add, (void *) 
                  &thread_data_array[t]);
          }
          else{
            rc = pthread_create(&threads[t], NULL, (void *)Print, (void *)
               &thread_data_array[t]);
          }

          if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
            }
        
    }
    
    sleep(10);
    printf("Exit the code");


    return 0;
}
