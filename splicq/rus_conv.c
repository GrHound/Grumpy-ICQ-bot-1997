/**  rus_conv.c  **/

#include "micq.h"
#ifdef _WIN32
 #include <winuser.h>
 /* OemToChar and CharToOemis here...*/
#endif
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef unsigned char uchar;

/* 19981206 alvar (Alexander Varin) added conversion of russian letter io */

uchar kw[] = {128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
              144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
              160,161,162,184,164,165,166,167,168,169,170,171,172,173,174,175,
              176,177,178,168,180,181,182,183,184,185,186,187,188,189,190,191,
              254,224,225,246,228,229,244,227,245,232,233,234,235,236,237,238,
              239,255,240,241,242,243,230,226,252,251,231,248,253,249,247,250,
              222,192,193,214,196,197,212,195,213,200,201,202,203,204,205,206,
              207,223,208,209,210,211,198,194,220,219,199,216,221,217,215,218};

uchar wk[] = {128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
              144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
              160,161,162,163,164,165,166,167,179,169,170,171,172,173,174,175,
              176,177,178,179,180,181,182,183,163,185,186,187,188,189,190,191,
              225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,
              242,243,244,245,230,232,227,254,251,253,255,249,248,252,224,241,
              193,194,215,199,196,197,214,218,201,202,203,204,205,206,207,208,
              210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,209};

/********************************************************
Russian language ICQ fix.
Usual Windows ICQ users do use Windows 1251 encoding but
unix users do use koi8 encoding, so we need to convert it.
This function will convert string from windows 1251 to koi8
or from koi8 to windows 1251.
Andrew Frolov dron@ilm.net
*********************************************************/
void rus_conv( char to[4], char *t_in )
{
   uchar *table;
   int i;

#ifdef _WIN32
   /* Jerom's code %) */
   char tempbuff[10000]; /* What happens if we use t_in twice??? */
   /* Does windows barf? it shouldn't but it shouldn't crash either */
   if ( Russian )
   {
      assert( strlen( t_in ) < 9999 );
      if(strcmp(to, "kw") == 0)
         OemToChar(t_in, tempbuff);
      /* If you have errors here try to change this function to
       OemToAnsi() */
      else
         CharToOem(t_in, tempbuff);
      /*  If you have errors here try to change this function to
       AnsiToOem() */
      strcpy(t_in, tempbuff);
   }
#else


/* 6-17-1998 by Linux_Dude
 * Moved initialization of table out front of 'if' block to prevent compiler
 * warning. Improved error message, and now return without performing string
 * conversion to prevent addressing memory out of range (table pointer would
 * previously have remained uninitialized ( = bad)).
*/

   table = wk;
   if(strcmp(to, "kw") == 0)
      table = kw;
   else if(strcmp(to, "wk") != 0)
   {
      fprintf(stderr, "\nError - \"to\" in call to rus_conv() is \"%s\", must " \
                      "be \"wk\" or \"kw\".\nWarning - Sending message " \
                      "without converting string.\n", to);
      return;
   }
      
/* End Linux_Dude's changes ;)
*/

   if ( Russian )
   {
      for (i=0;t_in[i]!=0;i++)
      {
         t_in[i] &= 0377;
         if(t_in[i] & 0200)
            t_in[i] = table[t_in[i] & 0177];
      }
   }
#endif
}
