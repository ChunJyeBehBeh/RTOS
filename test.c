 #include <stdio.h>
 
void main()
{
  unsigned char count=0x11,chan;
  chan = count&0x1f;
  printf("0x%x\n",chan);
  
}
