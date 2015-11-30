
/*

    Bit shift a four byte unsigned char as a unsigned long int. Practice code for figuring
out how to get time bytes from a NIST time server.
    Tested on Ubuntu14.04 32bit intel computer. 

    gcc -Wall bit_shift1.c -o bit_shift1

    C. Eric Cashon

*/

#include<stdio.h>

int main()
  {
    //Set max value for unsigned long int.
    unsigned long int max=~0;
    //Read bytes big endian with multiplication.
    unsigned char buffer1[]={0, 0, 0, 255};
    //Read bytes big endian with bit shift.
    unsigned char buffer2[]={255, 0, 0, 0};
    //Read bytes as little endian with multiplication.
    unsigned char buffer3[]={0, 0, 0, 255};
    //Read bytes as little endian with bit shift.
    unsigned char buffer4[]={255, 0, 0, 0};

    printf("Max %lu\n", max);

    printf("Buffer1 BE %lu\n",(((unsigned long int)buffer1[0])*256*256*256)+(((unsigned long int)buffer1[1])*256*256)+(((unsigned long int)buffer1[2])*256)+((unsigned long int)buffer1[3]));

    printf("Buffer2 BE %lu\n",((unsigned long int)buffer2[0]<<24)|((unsigned long int)buffer2[1]<<16)|((unsigned long int)buffer2[2]<<8)|buffer2[3]); 
  
    printf("Buffer3 LE %lu\n",((unsigned long int)buffer3[0])+(((unsigned long int)buffer3[1])*256)+(((unsigned long int)buffer3[2])*256*256)+(((unsigned long int)buffer3[3])*256*256*256));

    printf("Buffer4 LE %lu\n",((unsigned long int)buffer4[0])|((unsigned long int)buffer4[1]<<8)|((unsigned long int)buffer4[2]<<16)|((unsigned long int)buffer4[3]<<24)); 
       
    return 0;
  }
