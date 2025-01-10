#include "datatype.h"
#include <stdio.h>
#include <stdlib.h>

DWORD Descramble_cc( DWORD cc )
{
    DWORD a[6];
    
    a[1] = cc & 0x0001F000;
    a[2] = cc & 0x07C007C0;
    a[3] = cc & 0x003E0001;
    a[4] = cc & 0xF8000000;
    a[5] = cc & 0x0000083E;
    
    a[1] >>= 0x0C;
    a[2] >>= 0x01;
    a[3] <<= 0x0A;
    a[4] >>= 0x10;
    a[5] <<= 0x0F;
    
    return a[1] + a[2] + a[3] + a[4] + a[5];
}

DWORD Scramble_cc( DWORD cc )
{
    DWORD a[6];
    
    a[1] = cc & 0x0000001F;
    a[2] = cc & 0x03E003E0;
    a[3] = cc & 0xF8000400;
    a[4] = cc & 0x0000F800;
    a[5] = cc & 0x041F0000;
    
    a[1] <<= 0x0C;
    a[2] <<= 0x01;
    a[3] >>= 0x0A;
    a[4] <<= 0x10;
    a[5] >>= 0x0F;
    
    return a[1] + a[2] + a[3] + a[4] + a[5];
}

void main( void ) 
{
   DWORD i;
   
   
   
   i = 0x8f6d7950;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x8f0084c8;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0xe4f83cf4;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x869a6808;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0xb8891cbe;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0xbe1c89b8;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   printf( "*****************************\n" );
   i = 0x01306a70;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x15a2f638;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x2002ed58;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x2886d1c7;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x323f4a7a;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x3cf016d7;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x4682b100;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x594580f8;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x6587af50;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x72ba2750;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x77bd6728;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x7df88ef8;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x86911fbc;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x8985bfe1;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x8b747ca1;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x8c58c8a0;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0x9b3f02dc;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0xbc0d29bd;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0xc21dd23a;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0xcd3c0818;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0xd89cbf22;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0xe4727766;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0xeec9e4b7;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0xef103f70;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
   i = 0xfb4158c0;
   printf( "%08lX = %08lX\n", i, Descramble_cc( i ) );
}
