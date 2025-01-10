#include "micq.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#ifdef _WIN32
   #include <io.h>
   #define S_IRUSR        _S_IREAD
   #define S_IWUSR        _S_IWRITE
#endif
#ifdef UNIX
   #include <unistd.h>
   #include <termios.h>
#endif
#include "mreadline.h"

#ifdef UNIX
//char *strdup( const char * );
//int strcasecmp( const char *, const char * );
//int strncasecmp( const char *, const char *, size_t );
#endif

static BOOL No_Prompt=FALSE;

#define ADD_COLOR(a)      else if ( ! strncmp( str2, a , strlen( a ) ) ) \
      {                                                                 \
         if ( Color )                                                   \
            printf( a );                                                \
         str2 += strlen( a );                                           \
      }



/***************************************************************
Turns keybord echo off for the password
****************************************************************/
S_DWORD Echo_Off( void )
{
#ifdef UNIX
    struct termios attr; /* used for getting and setting terminal
                attributes */

    /* Now turn off echo */
    if (tcgetattr(STDIN_FILENO, &attr) != 0) return(-1);
        /* Start by getting current attributes.  This call copies
        all of the terminal paramters into attr */

    attr.c_lflag &= ~(ECHO);
        /* Turn off echo flag.  NOTE: We are careful not to modify any
        bits except ECHO */
    if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&attr) != 0) return(-2);
        /* Wait for all of the data to be printed. */
        /* Set all of the terminal parameters from the (slightly)
           modified struct termios */
        /* Discard any characters that have been typed but not yet read */
#endif
    return 0;
}


/***************************************************************
Turns keybord echo back on after the password
****************************************************************/
S_DWORD Echo_On( void )
{
#ifdef UNIX
    struct termios attr; /* used for getting and setting terminal
                attributes */

    if (tcgetattr(STDIN_FILENO, &attr) != 0) return(-1);

    attr.c_lflag |= ECHO;
    if(tcsetattr(STDIN_FILENO,TCSANOW,&attr) != 0) return(-1);
#endif
    return 0;
}

/**************************************************************
Same as fM_print but for FD_T's
***************************************************************/
void M_fdprint( FD_T fd, char *str, ... )
{
   va_list args;
   int k;
   char buf[2048]; /* this should big enough */
        
   assert( buf != NULL );
   assert( 2048 >= strlen( str ) );
   
   va_start( args, str );
   vsprintf( buf, str, args );
   k = write( fd, buf, strlen( buf ) );
   if ( k != strlen( buf ) )
   {
      perror(str);
      exit ( 10);
   }
   va_end( args );
}

/************************************************************
Prints the preformated sting to stdout.
Plays sounds if appropriate.
************************************************************/
static void M_prints( char *str )
{
   int i;
   
   for ( i=0; str[i] != 0; i++ )
   {
      if ( str[i] != '\a' )
         printf( "%c", str[i] );
      else if ( SOUND_ON == Sound )
         printf( "\a" );
      else if ( SOUND_CMD == Sound )
     system ( (char *) Sound_Str );
   }
}

/**************************************************************
M_print with colors.
***************************************************************/
void M_print( char *str, ... )
{
   va_list args;
   char buf[2048];
   char *str1, *str2;
   
   va_start( args, str );
#ifndef CURSES_UI
   vsprintf( buf, str, args );
   str2 = buf;
   while ( (void *) NULL != ( str1 = strchr( str2, '\x1b' ) ) )
   {
      str1[0] = 0;
      M_prints( str2 );
      str1[0] = 0x1B;
      str2 = str1;
      if ( FALSE ) {;}
      ADD_COLOR( NOCOL )
      ADD_COLOR( SERVCOL )
      ADD_COLOR( MESSCOL )
      ADD_COLOR( CONTACTCOL )
      ADD_COLOR( CLIENTCOL )
      else
      {
          str2++;
      }
   }
   M_prints( str2 );
#else
   #error No curses support included yet.
   #error You must add it yourself.
#endif
   va_end( args );
}

/***********************************************************
Reads a line of input from the file descriptor fd into buf
an entire line is read but no more than len bytes are 
actually stored
************************************************************/
int M_fdnreadln( FD_T fd, char *buf, size_t len )
{
   int i,j;
   char tmp;

   assert( buf != NULL );
   assert( len > 0 );
   tmp = 0;
   len--;
   for ( i=-1; ( tmp != '\n' )  ; )
   {
      if  ( ( i < len ) || ( i == -1 ) )
      {
         i++;
         j = read( fd, &buf[i], 1 );
         tmp = buf[i];
      }
      else
      {
         j = read( fd, &tmp, 1 );
      }
      assert( j != -1 );
      if ( j == 0 )
      {
         buf[i] =  0;
         return -1;
      }
   }
   if ( i < 1 )
   {
      buf[i] = 0;
   }
   else
   {
      if ( buf[i-1] == '\r' )
      {
         buf[i-1] = 0;
      }
      else
      {
         buf[i] = 0;
      }
   } 
   return 0;
}

/*****************************************************
Disables the printing of the next prompt.
useful for multipacket messages.
******************************************************/
void Kill_Prompt( void )
{
     No_Prompt = TRUE;
}

/*****************************************************
Displays the Micq prompt.  Maybe someday this will be 
configurable
******************************************************/
void Prompt( void )
{
//#ifndef USE_MREADLINE
#if 0
   if ( !No_Prompt ) 
#endif
      R_doprompt ( SERVCOL PROMPT_STR NOCOL );
#ifndef USE_MREADLINE
//#error test
      fflush( stdout );
#endif
   No_Prompt = FALSE;
}

/*****************************************************
Displays the Micq prompt.  Maybe someday this will be 
configurable
******************************************************/
void Soft_Prompt( void )
{
//#ifdef USE_MREADLINE
#if 1
   R_doprompt ( SERVCOL PROMPT_STR NOCOL );
   No_Prompt = FALSE;
#else
   if ( !No_Prompt ) {
      M_print( PROMPT_STR );
      fflush( stdout );
   } else {
     No_Prompt = FALSE;
   }
#endif
}

void Time_Stamp( void )
{
   struct tm *thetime;
   time_t p;
   
   p=time(NULL);
   thetime=localtime(&p);

   M_print( "%.02d:%.02d:%.02d",thetime->tm_hour,thetime->tm_min,thetime->tm_sec );
}
