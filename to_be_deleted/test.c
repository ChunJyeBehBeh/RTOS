 #include <stdio.h>
 
void main()
{
  unsigned char count=0x11,chan;
  chan = count&0x1f;
  int i;
  i = (int) count;
  printf("0x%x in decimal is %d\n",count,count);
  printf("Type casting to convert hex to dec: %d\n",i);
  
}
