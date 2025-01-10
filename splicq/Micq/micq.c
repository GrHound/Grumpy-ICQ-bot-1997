/*********************************************
**********************************************
This is the main ICQ file. Currently it
logs in and sits in a loop. It can receive
messages and keeps the connection alive.
Use crtl-break to exit.

This software is provided AS IS to be used in
whatever way you see fit and is placed in the
public domain.

Author : Matthew Smith April 19, 1998
Contributors : Nicolas Sahlqvist April 27, 1998
			   Ulf Hedlund (guru@slideware.com) April 28, 1998
            Michael Ivey May 4, 1998
            Michael Holzt May 5, 1998

Changes :
   4-28-98 support for WIN32 [UH]
   4-20-98 added variable time_delay between keep_alive packets mds
   4-20-98 added instant message from server support mds
   4-21-98 changed so that long ( 250+ characters ) messages work
            new maximum is ~900 which is hopefully big enough.
            When I know more about udp maybe I can come up with
            a general solution. mds I now think ICQ has a max that is
            smaller than this so everything is ok mds I now think that
            the icq client's maximum is arbitrary and can be ignored :)
   4-23-98 Added beginnings of a user interface
   4-26-98 Changed the version to 0.2a :)
   4-27-98 Nicco added feature to use nick names to log in
   5-05-98 Authorization Messages
   5-13-98 Added time stamps for most things.
   6-17-98 Changed condition on which we should send auto_reply message. Fryslan
   6-18-98 Added different auto reply messages for different status types see also ui.c and util.c Fryslan
   6-20-98 Added an alter command to alter your command names online. Fryslan
**********************************************
**********************************************/
#include "micq.h"
#include "datatype.h"
#include "msg_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#ifdef _WIN32
#include <conio.h>
#include <io.h>
#include <winsock2.h>
#include <time.h>
#else
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/wait.h>
#include "mreadline.h"
#endif
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "splicq.h"

BYTE Sound = SOUND_ON; /* Beeps on by default */
BYTE Sound_Str[150];  /* the command to run from the shell to play sound files */
BOOL Russian = FALSE; /* Do we do kio8-r <->Cp1251 codeset translation? */
BOOL Logging = TRUE;  /* Do we log messages to ~/micq_log?  This should probably have different levels */
BOOL Color = TRUE; /* Do we use ANSI color? */
BOOL Quit = FALSE;    /* set when it's time to exit the program */
BOOL Verbose = FALSE; /* this is displays extra debuging info */
BOOL serv_mess[ 1024 ]; /* used so that we don't get duplicate messages with the same SEQ */
WORD last_cmd[ 1024 ]; /* command issued for the first 1024 SEQ #'s */
/******************** if we have more than 1024 SEQ this will need some hacking */
WORD seq_num = 1;  /* current sequence number */
DWORD our_ip = 0x0100007f; /* localhost for some reason */
DWORD our_port; /* the port to make tcp connections on */
/************ We don't make tcp connections yet though :( */
DWORD UIN; /* current User Id Number */
BOOL Contact_List = FALSE; /* I think we always have a contact list now */
Contact_Member Contacts[ 100 ]; /* no more than 100 contacts max */
int Num_Contacts=0;
DWORD Current_Status=STATUS_OFFLINE;
DWORD BotMode=1;
Human Last_Probed_Rand_User = {0,0,"",""};
DWORD last_recv_uin=0;
char passwd[100];
char server[100];
DWORD set_status;
DWORD remote_port;
BOOL Done_Login=FALSE;
BOOL auto_resp=TRUE;
char auto_rep_str_dnd[450] = { "Don't page me, my head is hurting!" };
char auto_rep_str_away[450] = { "I told you I wasn't here!" };
char auto_rep_str_na[450] = { "Working, working always working..." };
char auto_rep_str_occ[450] = { "I am working on opening this beer so I am busy." };
char auto_rep_str_inv[450] = { "So you can see me, so you can't!" };
char message_cmd[16];
char info_cmd[16];
char quit_cmd[16];
char reply_cmd[16];
char again_cmd[16];
char add_cmd[16];

char list_cmd[16];
char away_cmd[16];
char na_cmd[16];
char dnd_cmd[16];
char online_cmd[16];
char occ_cmd[16];
char ffc_cmd[16];
char inv_cmd[16];
char status_cmd[16];
char auth_cmd[16];
char auto_cmd[16];
char change_cmd[16];
char search_cmd[16];
char save_cmd[16];
char alter_cmd[16];
char msga_cmd[16];
char url_cmd[16];
char update_cmd[16];
char rand_cmd[16];
char color_cmd[16];
char sound_cmd[16];

/*** auto away values ***/
int idle_val=0;
int idle_flag=0;
#define away_time 600

unsigned int next_resend;

/*/////////////////////////////////////////////
// Connects to hostname on port port
// hostname can be DNS or nnn.nnn.nnn.nnn
// write out messages to the FD aux */
int Connect_Remote( char *hostname, int port, FD_T aux )
{

   int conct, length;
   int sok;
   struct sockaddr_in sin;  /* used to store inet addr stuff */
   struct hostent *host_struct; /* used in DNS lookup */

#if 1
	sin.sin_addr.s_addr = inet_addr( hostname ); 
  	if ( sin.sin_addr.s_addr  == -1 ) /* name isn't n.n.n.n so must be DNS */
#else

	if ( inet_aton( hostname, &sin.sin_addr )  == 0 ) /* checks for n.n.n.n notation */
#endif
	{
	   host_struct = gethostbyname( hostname );/* name isn't n.n.n.n so must be DNS */
      if ( host_struct == NULL )
      {
         if ( Verbose )
         {
            M_fdprint( aux, "Shakespeare couldn't spell why should I?\n" );
            M_fdprint( aux, " Especially something like %s\n", hostname );
            /*herror( "Can't find hostname" );*/
         }
         return 0;
      }
	   sin.sin_addr = *((struct in_addr *)host_struct->h_addr);
   }
	sin.sin_family = AF_INET; /* we're using the inet not appletalk*/
	sin.sin_port = htons( port );	/* port */
	sok = socket( AF_INET, SOCK_DGRAM, 0 );/* create the unconnected socket*/
   if ( sok == -1 ) 
   {
      perror( "Socket creation failed" );
      exit( 1 );
   }   
   if ( Verbose )
   {
      M_fdprint( aux, "Socket created attempting to connect\n" );
   }
	conct = connect( sok, (struct sockaddr *) &sin, sizeof( sin ) );
	if ( conct == -1 )/* did we connect ?*/
	{
      if ( Verbose )
      {
   	   M_fdprint( aux, " Conection Refused on port %d at %s\n", port, hostname );
         #ifdef FUNNY_MSGS
            M_fdprint( aux, " D'oh!\n" );
         #endif
   	   perror( "connect" );
      }
	   return 0;
	}
   length = sizeof( sin ) ;
   getsockname( sok, (struct sockaddr *) &sin, &length );
   our_ip = sin.sin_addr.s_addr;
   our_port = sin.sin_port;
   if (Verbose )
   {
      #ifdef FUNNY_MSGS
         M_fdprint( aux, "Our port is %d, take her to sea Mr. Mordoch.\n", ntohs( sin.sin_port ) );
      #else
         M_fdprint( aux, "The port that will be used for tcp ( not yet implemented ) is %d\n", ntohs( sin.sin_port ) );
      #endif
   }
   if ( Verbose )
   {
      M_fdprint( aux, "Connected to %s, waiting for response\n", hostname );
   }
   return sok;
}


/******************************************
Handles packets that the server sends to us.
*******************************************/
void Handle_Server_Response( SOK_T sok )
{
   srv_net_icq_pak pak;
   static DWORD last_seq=-1;
   int s;
   
   s = SOCKREAD( sok, &pak.head.ver, sizeof( pak ) - 2  );
   if ( s < 0 )
   	return;
      
#if 0
   M_print( "Cmd : %04X\t",Chars_2_Word( pak.head.cmd ) );
   M_print( "Ver : %04X\t",Chars_2_Word( pak.head.ver ) );
   M_print( "Seq : %08X\t",Chars_2_DW( pak.head.seq ) );
   M_print( "Ses : %08X\n",Chars_2_DW( pak.head.session ) );
#endif
  if ( Chars_2_DW( pak.head.session ) != our_session ) {
     if ( Verbose ) {
       	R_undraw();
        M_print( "Got a bad session ID %08X with CMD %04X ignored.\n", 
            Chars_2_DW( pak.head.session ), Chars_2_Word( pak.head.cmd ) );
	R_redraw();
     }
     return;
  }
  /* !!! TODO make a check checksum routine to verify the packet further */
/*   if ( ( serv_mess[ Chars_2_Word( pak.head.seq2 ) ] ) && 
      ( Chars_2_Word( pak.head.cmd ) != SRV_NEW_UIN ) )*/
   if ( ( last_seq == Chars_2_DW( pak.head.seq ) ) && 
      ( Chars_2_Word( pak.head.cmd ) != SRV_NEW_UIN ) )
    {
      if ( Chars_2_Word( pak.head.cmd ) != SRV_ACK ) /* ACKs don't matter */
      {
         if ( Verbose ) {
       	    R_undraw();
            M_print( "\nIgnored a message cmd  %04x\n", Chars_2_Word( pak.head.cmd )  );
	    R_redraw();
	 }
         ack_srv( sok, Chars_2_DW( pak.head.seq ) ); /* LAGGGGG!! */
         return;
      }
   }
   if ( Chars_2_Word( pak.head.cmd ) != SRV_ACK )
   {
      serv_mess[ Chars_2_Word( pak.head.seq2 ) ] = TRUE;
      last_seq = Chars_2_DW( pak.head.seq );
      ack_srv( sok, Chars_2_DW( pak.head.seq ) );
   }
   Server_Response( sok, pak.data, s - ( sizeof( pak.head ) - 2 ), 
      Chars_2_Word( pak.head.cmd ), Chars_2_Word( pak.head.ver ),
      Chars_2_DW( pak.head.seq ), Chars_2_DW( pak.head.UIN ) );
}

/**********************************************
Verifies that we are in the correct endian
***********************************************/
void Check_Endian( void )
{
   int i;
   char passwd[10];
   
   passwd[0] = 1;
   passwd[1] = 0;
   passwd[2] = 0;
   passwd[3] = 0;
   passwd[4] = 0;
   passwd[5] = 0;
   passwd[6] = 0;
   passwd[7] = 0;
   passwd[8] = 0;
   passwd[9] = 0;
   i = *  ( DWORD *) passwd;
   if ( i == 1 )
   {
      M_print( "Using intel byte ordering." );
   }
   else
   {
      M_print( "Using motorola byte ordering." );
   }
}

int unseen_rand_user(int iuin) 
{
	FILE *fp;
	int i, iret;
	int had_it;
#define UIN_FILE 	"seen_rand_uin.dat"
	
	fp = fopen(UIN_FILE,"r");
	if(fp == NULL) {
		fp = fopen(UIN_FILE,"w");
		fprintf(fp,"%d\n", iuin);
		fclose(fp);
		iret = 1;
		
	} else {
		had_it = 0;
		while(fscanf(fp, "%d", &i) != EOF) {
			if(i == iuin) {
				had_it = 1;
				break;
			}
		}
		fclose(fp);
		if(! had_it) {
		   fp = fopen(UIN_FILE,"a");
		   fprintf(fp,"%d\n", iuin);
		   fclose(fp);
		   iret = 1;
		} else {
		   iret = 0;
		}
	}
	return(iret);
}

void make_first_call(char *sms)
{
   FILE *fp;
   char os_line[500];
   
   if(strlen(Last_Probed_Rand_User.nickname) != 0) {
       sprintf(sms,"Hi %s!", Last_Probed_Rand_User.nickname);
   } else {
       if(strlen(Last_Probed_Rand_User.firstname) != 0) {
       	  sprintf(sms,"Hi %s!", Last_Probed_Rand_User.firstname);
       } else {
       	  strcpy(sms,"Hi!");
       }
   }
   sprintf(os_line,"mkdir -p %s/%ld", LOGDIR, Last_Probed_Rand_User.uin);
   system(os_line);
   sprintf(os_line,"%s/%ld/info", LOGDIR, Last_Probed_Rand_User.uin);
   fp = fopen(os_line,"w");
   if(fp != NULL) {
   	fprintf(fp,"%ld\n", Last_Probed_Rand_User.uin);
   	fprintf(fp,"%s\n", Last_Probed_Rand_User.nickname);
   	fprintf(fp,"%s\n", Last_Probed_Rand_User.firstname);
   	fclose(fp);
   }
}

/******************************
Idle checking function
added by Warn Kitchen 1/23/99
******************************/
void Idle_Check( SOK_T sok )
{
   int tm, i, j, k;
   static int delta = 1 /* second */;
   static int minutes_rand_select = 5;
   static int minutes_read_ctl = 5;
   int micros = 1000000;
   int sec_per_min = 60;
   int rand_user_group = 2;
   static int mytime_seconds = 0;
   FILE *fp;
   char sms[200];
   
   if(BotMode) {
   	usleep(delta * micros);
   	mytime_seconds += delta;
   	
        /* Poll parameters from ctlfile */ 
  
   	if(mytime_seconds % (minutes_read_ctl * sec_per_min) == 0) {
   		fp = fopen("micq.ctl","r");
   		if(fp != NULL) {
   			if(fscanf(fp,"%d%d%d",&i,&j,&k) == 3) {
   				if(i < 1) i = 1; /* 1 sec. min resp delay */
   				if(j < 1) j = 1; /* 1 min. min randselect */
   				if(k < 0) k = 0;
   				
   				delta = i;
   				minutes_rand_select = j;
   				rand_user_group = k;
   			}
   			fclose(fp);
   			fprintf(stderr
   			   ,"T=%ds .ctl: delta=%d minrandsel=%d randg=%d\n"
   			              ,mytime_seconds
   			              ,delta, minutes_rand_select
   			              ,rand_user_group);
   		        fflush(stderr);
   		}
        }
        
        /* Request a random icq user search in the selected group */

   	if(mytime_seconds % (minutes_rand_select * sec_per_min) == 0) {
   	        fprintf(stderr
   			   ,"T=%ds rndreq minrandsel=%d randg=%d\n"
   			              ,mytime_seconds
   			              ,minutes_rand_select
   			              ,rand_user_group);
   		fflush(stderr);
   		icq_rand_user_req(sok,rand_user_group);
   		
   	} else {
   		if(Last_Probed_Rand_User.uin > 0 &&
   		   Last_Probed_Rand_User.ready) {
   			if(unseen_rand_user(Last_Probed_Rand_User.uin)) {
                           M_print( "rand msg to rand user" );
	   		   usleep(1 * micros);
	   		   make_first_call(sms);
                           icq_sendmsg( sok, Last_Probed_Rand_User.uin
                                           , sms, NORM_MESS );
       			}
      		}
   		if(Last_Probed_Rand_User.ready) {
   			Last_Probed_Rand_User.ready = 0;
   		}
   	}
   	return;
   }

   tm = ( time( NULL ) - idle_val );
   if ( ( Current_Status == STATUS_AWAY || Current_Status == STATUS_NA )
           && tm < away_time && idle_flag == 1) {
      icq_change_status(sok,STATUS_ONLINE);
      R_undraw ();
      M_print( "\nAuto-Changed status to " );
      Print_Status(Current_Status);
      M_print( " " );
      Time_Stamp();
      M_print( "\n" );
      R_redraw ();
      idle_flag=0;
      return;
   }
   if ( (Current_Status == STATUS_AWAY) && (tm >= (away_time*2)) && (idle_flag == 1) ) {
      icq_change_status(sok,STATUS_NA);
      R_undraw ();
      M_print( "\nAuto-Changed status to " );
      Print_Status(Current_Status);
      M_print( " " );
      Time_Stamp();
      M_print( "\n" );
      R_redraw ();
      return;
   }
   if ( Current_Status != STATUS_ONLINE && Current_Status != STATUS_FREE_CHAT ) {
      return;
   }
   if(tm>=away_time) {
      icq_change_status(sok,STATUS_AWAY); 
      R_undraw ();
      M_print( "\nAuto-Changed status to " );
      Print_Status(Current_Status);
      M_print( " " );
      Time_Stamp();
      M_print( "\n" );
      R_redraw ();
      idle_flag=1;
   }
   return;
}

/******************************
Main function connects gets UIN
and passwd and logins in and sits
in a loop waiting for server responses.
******************************/
int main( int argc, char *argv[] )
{
   int sok;
   int i;
   int next;
   int time_delay = 120;
   struct timeval tv;
   fd_set readfds;
#ifdef _WIN32
   WSADATA wsaData;
#endif

   setbuf (stdout, NULL); /* Don't buffer stdout */
   M_print( SERVCOL "Matt's ICQ clone " NOCOL "compiled on %s %s\n" SERVCOL " Version " MICQ_VERSION NOCOL "\n", __TIME__, __DATE__ );
#ifdef FUNNY_MSGS
   M_print( "No Mirabilis client was maimed, hacked, tortured, sodomized or otherwise harmed\nin the making of this utility.\n" );
#else
   M_print( "This program was made without any help from Mirabilis or their consent.\n" );
   M_print( "No reverse engineering or decompilation of any Mirabilis code took place\nto make this program.\n" );
#endif
   Get_Config_Info();
   srand( time( NULL ) );
   if ( !strcmp( passwd,"" ) )
   {
      M_print( "Enter password : " );
      Echo_Off();
      M_fdnreadln(STDIN, passwd, sizeof(passwd));
      Echo_On();
   }
   memset( serv_mess, FALSE, 1024 );
   Initialize_Msg_Queue();
   if (argc > 1 )
   {
      for ( i=1; i< argc; i++ )
      {
         if ( argv[i][0] != '-' )
         ;
         else if ( (argv[i][1] == 'v' ) || (argv[i][1] == 'V' ) )
         {
            Verbose++;
         }
      }
   }
   Check_Endian();
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
   sok = Connect_Remote( server, remote_port, STDERR );
   if ( ( sok == -1 ) || ( sok == 0 ) ) 
   {
   	M_print( "Couldn't establish connection\n" );
   	exit( 1 );
   }
   Login( sok, UIN, &passwd[0], our_ip, our_port, set_status );
   next = time( NULL );
   idle_val = time( NULL );
   next += 120;
   next_resend = 10;
   R_init ();
   M_print ("\n");
   Prompt();
   for ( ; !Quit; )
   {
      Idle_Check( sok );
#ifdef UNIX
      tv.tv_sec = 2;
      tv.tv_usec = 500000;
#else
	  tv.tv_sec = 0;
      tv.tv_usec = 100000;
#endif

      FD_ZERO(&readfds);
      FD_SET(sok, &readfds);
#ifndef _WIN32
      FD_SET(STDIN, &readfds);
#endif

      /* don't care about writefds and exceptfds: */
      select(sok+1, &readfds, NULL, NULL, &tv);

      if (FD_ISSET(sok, &readfds))
          Handle_Server_Response( sok );
#if 0
      else
          M_print( "\a" );
#endif      
#if _WIN32
	  if (_kbhit())		/* sorry, this is a bit ugly...   [UH]*/
#else      
	  if (FD_ISSET( STDIN, &readfds ) )
#endif
      {
         idle_val = time( NULL );
/*	 M_print( "%04X\n", sok );*/
	 if (R_process_input ())
		Get_Input( sok );
      }

      if ( time( NULL ) > next )
      {
         next = time( NULL ) + time_delay;
         Keep_Alive( sok );
      }

      if ( time( NULL ) > next_resend )
      {
        Do_Resend( sok );
      }
#ifdef UNIX
      while(waitpid(-1,NULL,WNOHANG) > 0); /* clean up child processes */
#endif
   }
   Quit_ICQ( sok );
   return 0;
}


  
