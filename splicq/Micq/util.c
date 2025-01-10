/*********************************************
**********************************************
This is a file of general utility functions useful
for programming in general and icq in specific

This software is provided AS IS to be used in
whatever way you see fit and is placed in the
public domain.

Author : Matthew Smith April 23, 1998
Contributors :  airog (crabbkw@rose-hulman.edu) May 13, 1998


Changes :
  6-18-98 Added support for saving auto reply messages. Fryslan
 
**********************************************
**********************************************/
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
   #include "mreadline.h"
#endif

#ifdef UNIX
//char *strdup( const char * );
//int strcasecmp( const char *, const char * );
//int strncasecmp( const char *, const char *, size_t );
#endif

typedef struct 
{
   const char *name;
   WORD code;
} COUNTRY_CODE;


static COUNTRY_CODE Country_Codes[] = { { USA_COUNTRY_STR, 1 },
                                 { Afghanistan_COUNTRY_STR, 93 },
                                 { Albania_COUNTRY_STR, 355 },
                                 { Algeria_COUNTRY_STR, 213 },
                                 { American_Samoa_COUNTRY_STR, 684 },
                                 { Andorra_COUNTRY_STR, 376 },
                                 { Angola_COUNTRY_STR, 244 },
                                 { Anguilla_COUNTRY_STR, 101 },
                                 { Antigua_COUNTRY_STR, 102 },
                                 { Argentina_COUNTRY_STR, 54 },
                                 { Armenia_COUNTRY_STR, 374 },
                                 { Aruba_COUNTRY_STR, 297 },
                                 { Ascention_Island_COUNTRY_STR, 274 },
                                 { Australia_COUNTRY_STR, 61 },
                                 { Australian_Antartic_Territory_COUNTRY_STR, 6721 },
                                 { Austria_COUNTRY_STR, 43 },
                                 { Azerbaijan_COUNTRY_STR, 934 },
                                 { Bahamas_COUNTRY_STR, 103 },
                                 { Bahrain_COUNTRY_STR, 973 },
                                 { Bangladesh_COUNTRY_STR, 880 },
                                 { Barbados_COUNTRY_STR, 104 },
                                 { Belarus_COUNTRY_STR, 375 },
                                 { Belgium_COUNTRY_STR, 32 },
                                 { Belize_COUNTRY_STR, 501 },
                                 { Benin_COUNTRY_STR, 229 },
                                 { Bermuda_COUNTRY_STR, 105 },
                                 { Bhutan_COUNTRY_STR, 975 },
                                 { Bolivia_COUNTRY_STR, 591 },
                                 { Bosnia_Herzegovina_COUNTRY_STR, 387 },
                                 { Botswana_COUNTRY_STR, 267 },
                                 { Brazil_COUNTRY_STR, 55 },
                                 { British_Virgin_Islands_COUNTRY_STR, 106 },
                                 { Brunei_COUNTRY_STR, 673 },
                                 { Bulgaria_COUNTRY_STR, 359 },
                                 { Burkina_Faso_COUNTRY_STR, 226 },
                                 { Burundi_COUNTRY_STR, 257 },
                                 { Cambodia_COUNTRY_STR, 855 },
                                 { Cameroon_COUNTRY_STR, 237 },
                                 { Canada_COUNTRY_STR, 107 },
                                 { Cape_Verde_Islands_COUNTRY_STR, 238 },
                                 { Cayman_Islands_COUNTRY_STR, 108},
                                 { Central_African_Republic_COUNTRY_STR, 236},
                                 { Chad_COUNTRY_STR, 235},
                                 { Christmas_Island_COUNTRY_STR, 672},
                                 { Cocos_Keeling_Islands_COUNTRY_STR, 6101},
                                 { Comoros_COUNTRY_STR, 2691},
                                 { Congo_COUNTRY_STR, 242},
                                 { Cook_Islands_COUNTRY_STR, 682},
                                 { Chile_COUNTRY_STR, 56 },
                                 { China_COUNTRY_STR, 86 },
                                 { Columbia_COUNTRY_STR, 57 },
                                 { Costa_Rice_COUNTRY_STR, 506 },
                                 { Croatia_COUNTRY_STR, 385 }, /* Observerd */
                                 { Cuba_COUNTRY_STR, 53 },
                                 { Cyprus_COUNTRY_STR, 357 },
                                 { Czech_Republic_COUNTRY_STR, 42 },
                                 { Denmark_COUNTRY_STR, 45 },
                                 { Diego_Garcia_COUNTRY_STR, 246},
                                 { Djibouti_COUNTRY_STR, 253},
                                 { Dominica_COUNTRY_STR, 109},
                                 { Dominican_Republic_COUNTRY_STR, 110},
                                 { Ecuador_COUNTRY_STR, 593 },
                                 { Egypt_COUNTRY_STR, 20 },
                                 { El_Salvador_COUNTRY_STR, 503 },
                                 { Equitorial_Guinea_COUNTRY_STR, 240},
                                 { Eritrea_COUNTRY_STR, 291},
                                 { Estonia_COUNTRY_STR, 372},
                                 { Ethiopia_COUNTRY_STR, 251 },
                                 { Former_Yugoslavia_COUNTRY_STR, 389},
                                 { Faeroe_Islands_COUNTRY_STR, 298},
                                 { Falkland_Islands_COUNTRY_STR, 500},
                                 { Federated_States_of_Micronesia_COUNTRY_STR, 691 },
                                 { Fiji_COUNTRY_STR, 679 },
                                 { Finland_COUNTRY_STR, 358 },
                                 { France_COUNTRY_STR, 33 },
                                 { French_Antilles_COUNTRY_STR, 596 },  /* Leave it */
                                 { French_Antilles_COUNTRY_STR, 5901 }, /* Either on or the other is right :) */
                                 { French_Guiana_COUNTRY_STR, 594 },
                                 { French_Polynesia_COUNTRY_STR, 689 },
                                 { Gabon_COUNTRY_STR, 241 },
                                 { Gambia_COUNTRY_STR, 220 },
                                 { Georgia_COUNTRY_STR, 995 },
                                 { Germany_COUNTRY_STR, 49 },
                                 { Ghana_COUNTRY_STR, 233 },
                                 { Gibraltar_COUNTRY_STR, 350 },
                                 { Greece_COUNTRY_STR, 30 },
                                 { Greenland_COUNTRY_STR, 299 },
                                 { Grenada_COUNTRY_STR, 111 },
                                 { Guadeloupe_COUNTRY_STR, 590 },
                                 { Guam_COUNTRY_STR, 671 },
                                 { Guantanomo_Bay_COUNTRY_STR, 5399 },
                                 { Guatemala_COUNTRY_STR, 502 },
                                 { Guinea_COUNTRY_STR, 224 },
                                 { Guinea_Bissau_COUNTRY_STR, 245 },
                                 { Guyana_COUNTRY_STR, 592 },
                                 { Haiti_COUNTRY_STR, 509 },
                                 { Honduras_COUNTRY_STR, 504 },
                                 { Hong_Kong_COUNTRY_STR, 852 },
                                 { Hungary_COUNTRY_STR, 36 },
                                 { Iceland_COUNTRY_STR, 354 },
                                 { India_COUNTRY_STR, 91 },
                                 { Indonesia_COUNTRY_STR, 62 },
                                 { INMARSAT_COUNTRY_STR, 870 },
                                 { INMARSAT_Atlantic_East_COUNTRY_STR, 870 },
                                 { Iran_COUNTRY_STR, 98 },
                                 { Iraq_COUNTRY_STR, 964 },
                                 { Ireland_COUNTRY_STR, 353 },
                                 { Israel_COUNTRY_STR, 972 },
                                 { Italy_COUNTRY_STR, 39 },
                                 { Ivory_Coast_COUNTRY_STR, 225 },
                                 { Japan_COUNTRY_STR, 81 },
                                 { Jordan_COUNTRY_STR, 962 },
                                 { Kenya_COUNTRY_STR, 254 },
                                 { South_Korea_COUNTRY_STR, 82 },
                                 { Kuwait_COUNTRY_STR, 965 },
                                 { Liberia_COUNTRY_STR, 231 },
                                 { Libya_COUNTRY_STR, 218 },
                                 { Liechtenstein_COUNTRY_STR, 4101 },
                                 { Luxembourg_COUNTRY_STR, 352 },
                                 { Malawi_COUNTRY_STR, 265 },
                                 { Malaysia_COUNTRY_STR, 60 },
                                 { Mali_COUNTRY_STR, 223 },
                                 { Malta_COUNTRY_STR, 356 },
                                 { Mexico_COUNTRY_STR, 52 },
                                 { Monaco_COUNTRY_STR, 33 },
                                 { Morocco_COUNTRY_STR, 212 },
                                 { Namibia_COUNTRY_STR, 264 },
                                 { Nepal_COUNTRY_STR, 977 },
                                 { Netherlands_COUNTRY_STR, 31 },
                                 { Netherlands_Antilles_COUNTRY_STR, 599 },
                                 { New_Caledonia_COUNTRY_STR, 687 },
                                 { New_Zealand_COUNTRY_STR, 64 },
                                 { Nicaragua_COUNTRY_STR, 505 },
                                 { Nigeria_COUNTRY_STR, 234 },
                                 { Norway_COUNTRY_STR, 47 }, 
                                 { Oman_COUNTRY_STR, 968 },
                                 { Pakistan_COUNTRY_STR, 92 },
                                 { Panama_COUNTRY_STR, 507 },
                                 { Papua_New_Guinea_COUNTRY_STR, 675 },
                                 { Paraguay_COUNTRY_STR, 595 },
                                 { Peru_COUNTRY_STR, 51 },
                                 { Philippines_COUNTRY_STR, 63 },
                                 { Poland_COUNTRY_STR, 48 },
                                 { Portugal_COUNTRY_STR, 351 },
                                 { Qatar_COUNTRY_STR, 974 },
                                 { Romania_COUNTRY_STR, 40 },
                                 { Russia_COUNTRY_STR, 7 },
                                 { Saipan_COUNTRY_STR, 670 },
                                 { San_Marino_COUNTRY_STR, 39 },
                                 { Saudia_Arabia_COUNTRY_STR, 966 },
                                 { Senegal_COUNTRY_STR, 221},
                                 { Singapore_COUNTRY_STR, 65 },
                                 { Slovakia_COUNTRY_STR, 42 },
                                 { South_Africa_COUNTRY_STR, 27 },
                                 { Spain_COUNTRY_STR, 34 },
                                 { Sri_Lanka_COUNTRY_STR, 94 },
                                 { Suriname_COUNTRY_STR, 597 },
                                 { Sweden_COUNTRY_STR, 46 },
                                 { Switzerland_COUNTRY_STR, 41 },
                                 { Taiwan_COUNTRY_STR, 886 },
                                 { Tanzania_COUNTRY_STR, 255 },
                                 { Thailand_COUNTRY_STR, 66 },
                                 { Tunisia_COUNTRY_STR, 216 },
                                 { Turkey_COUNTRY_STR, 90 },
                                 { United_Arab_Emirates_COUNTRY_STR, 971 },
                                 { Uruguay_COUNTRY_STR, 598 },
                                 { UK_COUNTRY_STR, 0x2c },
                                 { Ukraine_COUNTRY_STR, 380 },
                                 { Vatican_City_COUNTRY_STR, 39 },
                                 { Venezuela_COUNTRY_STR, 58 },
                                 { Vietnam_COUNTRY_STR, 84 },
                                 { Yemen_COUNTRY_STR, 967 },
                                 { Yugoslavia_COUNTRY_STR, 38 },
                                 { Zaire_COUNTRY_STR, 243 },
                                 { Zimbabwe_COUNTRY_STR, 263 },
#ifdef FUNNY_MSGS
                                 {NON_COUNTRY_FUNNY_STR, 0 },
                                 {NON_COUNTRY_FUNNY_STR, 0xffff } };
#else
                                 {NON_COUNTRY_STR, 0 },
                                 {NON_COUNTRY_STR, 0xffff } };
#endif


const char *Get_Country_Name( int code )
{
   int i;
   
   for ( i = 0; Country_Codes[i].code != 0xffff; i++)
   {
      if ( Country_Codes[i].code == code )
      {
         return Country_Codes[i].name;
      }
   }
   if ( Country_Codes[i].code == code )
   {
      return Country_Codes[i].name;
   }
   return NULL;
}

/********************************************
returns a string describing the status or
a NULL if no such string exists
*********************************************/
char *Convert_Status_2_Str( int status )
{
   if ( STATUS_OFFLINE == status ) /* this because -1 & 0xFFFF is not -1 */
   {
      return "Offline";
   }
   
   switch ( status & 0x1ff )
   {
   case STATUS_ONLINE:
      return STATUS_ONLINE_STR;
      break;
   case STATUS_DND_99 :
   case STATUS_DND:
      return STATUS_DND_STR;
      break;
   case STATUS_AWAY:
      return STATUS_AWAY_STR;
      break;
   case STATUS_OCCUPIED_MAC:
   case STATUS_OCCUPIED:
      return STATUS_OCCUPIED_STR;
      break;
   case STATUS_NA:
   case STATUS_NA_99:
      return STATUS_NA_STR;
      break;
   case STATUS_INVISIBLE:
      return STATUS_INVISIBLE_STR;
      break;
   case STATUS_FREE_CHAT:
      return STATUS_FFC_STR;
      break;
   default :
      return NULL;
      break;
   }
}


/********************************************
Prints a informative string to the screen.
describing the command
*********************************************/
void Print_CMD( WORD cmd )
{
   switch ( cmd )
   {
   case CMD_KEEP_ALIVE:
      M_print( "Keep Alive" );
      break;
   case CMD_KEEP_ALIVE2:
      M_print( "Secondary Keep Alive" );
      break;
   case CMD_CONT_LIST:
      M_print( "Contact List" );
      break;
   case CMD_INVIS_LIST:
      M_print( "Invisible List" );
      break;
   case CMD_VIS_LIST:
      M_print( "Visible List" );
      break;
   case CMD_RAND_SEARCH:
      M_print( "Random Search" );
      break;
   case CMD_RAND_SET:
      M_print( "Set Random" );
      break;
   case CMD_ACK_MESSAGES:
      M_print( "Delete Server Messages" );
      break;
   case CMD_LOGIN_1:
      M_print( "Finish Login" );
      break;
   case CMD_LOGIN:
      M_print( "Login" );
      break;
   case CMD_SENDM:
      M_print( "Send Message" );
      break;
   case CMD_INFO_REQ:
      M_print( "Info Request" );
      break;
   case CMD_EXT_INFO_REQ:
      M_print( "Extended Info Request" );
      break;
   default :
      M_print( "%04X", cmd );
      break;
   }
}

/********************************************
prints out the status of new_status as a string
if possible otherwise as a hex number
*********************************************/
void Print_Status( DWORD new_status  )
{
   if ( Convert_Status_2_Str( new_status ) != NULL )
   {
      M_print( "%s", Convert_Status_2_Str( new_status ) );
      if ( Verbose )
         M_print( " %06X",( WORD ) ( new_status >> 8 ) );
   }
   else
   {
      M_print( "%08lX", new_status );
   }
}

/**********************************************
 * Returns the nick of a UIN if we know it else
 * it will return Unknow UIN
 **********************************************/
char *UIN2nick( DWORD uin)
{
   int i;
    
   for ( i=0; i < Num_Contacts; i++ )
   {
     if ( Contacts[i].uin == uin )
        break;
   }
    
   if ( i == Num_Contacts )
   {
      return NULL;
   }
   else
   {
      return Contacts[i].nick;
   }
}

/**********************************************
Prints the name of a user or there UIN if name
is not know.
***********************************************/
int Print_UIN_Name( DWORD uin )
{
   int i;
   
   for ( i=0; i < Num_Contacts; i++ )
   {
      if ( Contacts[i].uin == uin )
         break;
   }

   if ( i == Num_Contacts )
   {
      M_print( CLIENTCOL "%lu" NOCOL, uin );
      return -1 ;
   }
   else
   {
      M_print( "%s%s%s", CONTACTCOL, Contacts[i].nick, NOCOL );
      return i;
   }
}

/**********************************************
Returns the contact list with uin
***********************************************/
CONTACT_PTR UIN2Contact( DWORD uin )
{
   int i;
   
   for ( i=0; i < Num_Contacts; i++ )
   {
      if ( Contacts[i].uin == uin )
         break;
   }

   if ( i == Num_Contacts )
   {
      return (CONTACT_PTR) NULL ;
   }
   else
   {
      return &Contacts[i];
   }
}

/*********************************************
Converts a nick name into a uin from the contact
list.
**********************************************/
DWORD nick2uin( char *nick )
{
   int i;
   BOOL non_numeric=FALSE;
   
   for ( i=0; i< Num_Contacts; i++ )
   {
      if ( ! strncasecmp( nick, Contacts[i].nick, 19  ) )
      {
         if ( (S_DWORD) Contacts[i].uin > 0 )
            return Contacts[i].uin;
         else
            return -Contacts[i].uin; /* alias */
      }
   }
   for ( i=0; i < strlen( nick ); i++ )
   {
      if ( ! isdigit( (int) nick[i] ) )
      {
         non_numeric=TRUE;
         break;
      }
   }
   if ( non_numeric )
      return -1; /* not found and not a number */
   else
      return atoi( nick );
}

/**************************************************
Automates the process of creating a new user.
***************************************************/
void Init_New_User( void )
{
   SOK_T sok; 
   srv_net_icq_pak pak;
   int s;
   struct timeval tv;
   fd_set readfds;
#ifdef _WIN32
   WSADATA wsaData;
#endif
      
#ifdef _WIN32
   i = WSAStartup( 0x0101, &wsaData );
   if ( i != 0 ) {
#ifdef FUNNY_MSGS
        perror("Windows Sockets broken blame Bill -");
#else
        perror("Sorry, can't initialize Windows Sockets...");
#endif
        exit(1);
   }
#endif
   M_print( "\nCreating Connection...\n");
   sok = Connect_Remote( server, remote_port, STDERR );
   if ( ( sok == -1 ) || ( sok == 0 ) ) 
   {
       M_print( "Couldn't establish connection\n" );
       exit( 1 );
   }
   M_print( "Sending Request...\n" );
   reg_new_user( sok, passwd );
   for ( ; ; )
   {
#ifdef UNIX
      tv.tv_sec = 3;
      tv.tv_usec = 500000;
#else
      tv.tv_sec = 0;
      tv.tv_usec = 100000;
#endif

      FD_ZERO(&readfds);
      FD_SET(sok, &readfds);

      /* don't care about writefds and exceptfds: */
      select(sok+1, &readfds, NULL, NULL, &tv);
      M_print( "Waiting for response....\n" );
      if (FD_ISSET(sok, &readfds))
      {
         s = SOCKREAD( sok, &pak.head.ver, sizeof( pak ) - 2  );
         if ( Chars_2_Word( pak.head.cmd ) == SRV_NEW_UIN )
         {
            UIN = Chars_2_DW( pak.head.UIN );
            M_print( "\nYour new UIN is %s%ld%s!\n",SERVCOL, UIN, NOCOL );
            return;
         }
         else
         {
/*            Hex_Dump( &pak.head.ver, s );*/
         }
      }
      reg_new_user( sok, passwd );
   }
}


void Print_IP( DWORD uin )
{
   int i;
#if 0
   struct in_addr sin;
#endif
   
   for ( i=0; i< Num_Contacts; i++ )
   {
      if ( Contacts[i].uin == uin )
      {
         if ( * (DWORD *)Contacts[i].current_ip != -1L )
         {
           M_print( "%d.%d.%d.%d", Contacts[i].current_ip[0],
                                   Contacts[i].current_ip[1],
                                   Contacts[i].current_ip[2],
                                   Contacts[i].current_ip[3] );
         }
         else
         {
            M_print( "unknown" );
         }
         return;
      }
   }
   M_print( "unknown" );
}

/************************************************
Gets the TCP port of the specified UIN
************************************************/
DWORD Get_Port( DWORD uin )
{
   int i;
   
   for ( i=0; i< Num_Contacts; i++ )
   {
      if ( Contacts[i].uin == uin )
      {
         return Contacts[i].port;
      }
   }
   return -1L;
}

/********************************************
Converts an intel endian character sequence to
a DWORD
*********************************************/
DWORD Chars_2_DW( unsigned char *buf )
{
   DWORD i;
   
   i= buf[3];
   i <<= 8;
   i+= buf[2];
   i <<= 8;
   i+= buf[1];
   i <<= 8;
   i+= buf[0];
   
   return i;
}

/********************************************
Converts an intel endian character sequence to
a WORD
*********************************************/
WORD Chars_2_Word( unsigned char *buf )
{
   WORD i;
   
   i= buf[1];
   i <<= 8;
   i += buf[0];
   
   return i;
}

/********************************************
Converts a DWORD to
an intel endian character sequence 
*********************************************/
void DW_2_Chars( unsigned char *buf, DWORD num )
{
   buf[3] = ( unsigned char ) ((num)>>24)& 0x000000FF;
   buf[2] = ( unsigned char ) ((num)>>16)& 0x000000FF;
   buf[1] = ( unsigned char ) ((num)>>8)& 0x000000FF;
   buf[0] = ( unsigned char ) (num) & 0x000000FF;
}

/********************************************
Converts a WORD to
an intel endian character sequence 
*********************************************/
void Word_2_Chars( unsigned char *buf, WORD num )
{
   buf[1] = ( unsigned char ) (((unsigned)num)>>8) & 0x00FF;
   buf[0] = ( unsigned char ) ((unsigned)num) & 0x00FF;
}

/*************************************************************************
 *      Function: log_event
 *      Purpose: Log the event provided to the log with a time stamp.
 *      Andrew Frolov dron@ilm.net
 *      6-20-98 Added names to the logs. Fryslan
 *************************************************************************/
int log_event( int type, char *str, ... )
{
   FILE    *msgfd;
   va_list args;
   int k;
   char buf[2048]; /* this should big enough */
   char    buffer[256];
   time_t  timeval;
   char *path;
   char *home;

   if ( ! Logging )
      return 0;

   
   timeval = time(0);
   va_start( args, str );
   sprintf( buf, "\n%-24.24s ", ctime(&timeval) );
   vsprintf( &buf[ strlen( buf ) ], str, args );

      
#ifdef _WIN32
   path = ".\\";
#endif

#ifdef UNIX
   home = getenv( "HOME" );
   path = malloc( strlen( home ) + 2 );
   strcpy( path, home );
   if ( path[ strlen( path ) - 1 ] != '/' )
      strcat( path, "/" );
#endif

#ifdef __amigaos__
   path = "PROGDIR:";
#endif

   strcpy( buffer, path );
   strcat( buffer, "micq_log" );


   if( ( msgfd = fopen(buffer, "a") ) == (FILE *) NULL ) 
   {
           fprintf(stderr, "Couldn't open %s for logging\n",
                            buffer);
           return(-1);
   }
/*    if ( ! strcasecmp(UIN2nick(uin),"Unknow UIN"))
       fprintf(msgfd, "\n%-24.24s %s %ld\n%s\n", ctime(&timeval), desc, uin, msg);
    else
       fprintf(msgfd, "\n%-24.24s %s %s\n%s\n", ctime(&timeval), desc, UIN2nick(uin), msg);*/

   k = fwrite( buf, 1, strlen( buf ), msgfd );
   if ( k != strlen( buf ) )
   {
       perror( "Log file write error" );
      return -1;
   }
   va_end( args );
     
   fclose(msgfd);
#ifdef UNIX
   chmod( buffer, 0600 );
   free( path );
#endif
   return(0);
}

/*************************************************
 clears the screen 
**************************************************/
void clrscr(void)
{
#ifdef UNIX
    system( "clear" );
#else
#ifdef _WIN32
    system( "cls" );
#else
    int x;
    char newline = '\n';    

     for(x = 0; x<=25; x++)
        M_print("%c",newline);
#endif
#endif
}

/************************************************************
Displays a hex dump of buf on the screen.
*************************************************************/
void Hex_Dump( void *buffer, size_t len )
{
      int i;
      int j;
      int k;
      char *buf;
      
      buf = buffer;
      if ( ( len < 0 ) || ( len > 1000 ) ) {
         M_print( "Ack!!!!!!  %d\a\n" , len );
	 return;
      }
      assert( len > 0 );
      assert( len < 1000 );
      if ( len < 0 )
          return;
      for ( i=0 ; i < len; i++ )
      {
         M_print( "%02x ", ( unsigned char ) buf[i] );
         if ( ( i & 15 ) == 15 )
         {
            M_print( "  " );
            for ( j = 15; j >= 0; j-- )
            {
               if ( buf[i-j] > 31 )
                  M_print( "%c", buf[i-j] );
               else
                  M_print( "." );
               if ( ( (i-j) & 3 ) == 3 )
                  M_print( " " );
            }
            M_print( "\n" );
         }
         else if ( ( i & 7 ) == 7 )
            M_print( "- " );
         else if ( ( i & 3 ) == 3 )
            M_print( "  " );
      }
      for ( k = i % 16; k <16; k++  )
      {
         M_print( "   " );
         if ( ( k & 7 ) == 7 )
            M_print( "  " );
         else if ( ( k & 3 ) == 3 )
            M_print( "  " );
      }
    /*  M_print( "  " );*/
      for ( j = i % 16; j > 0; j-- )
      {
         if ( buf[i-j] > 31 )
            M_print( "%c", buf[i-j] );
         else
            M_print( "." );
         if ( ( (i-j) & 3 ) == 3 )
            M_print( " " );
      }
/*      M_print( "\n" );*/
}
