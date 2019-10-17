#include <stdio.h>

int main(){
	int a;
	int value;
	printf("Enter integer: ");
	if(scanf("%d",&a)== 1)
	{
		value=getchar();
//		printf("buffer --- %d\n",value);
		if(value !='\n')
		{
			printf("Invalid Input end with alphabet.\n");			// try input: 32a
		}
		else
		{
			printf("Correct Input\n");								// try input: 32
			printf("--- %d\n",a);
		}	
	}
	else{
		printf("Invalid Input start with alphabet.\n");				// try input: a32
	}
	return 0;
}
