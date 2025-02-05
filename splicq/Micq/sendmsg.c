/*
Send Message Function for ICQ... 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Author : zed@mentasm.com
*/
#include "datatype.h"
#include "micq.h"
#include "msg_queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#ifdef _WIN32
  #include <winsock2.h>
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/socket.h>
#endif
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "mreadline.h"

/*unsigned int next_resend;*/

DWORD our_session;

static size_t SOCKWRITE_LOW( SOK_T sok, void * ptr, size_t len );

/* Historical */
void Initialize_Msg_Queue()
{
    msg_queue_init();
}

/****************************************************************
Checks if packets are waiting to be resent and sends them.
*****************************************************************/
void Do_Resend( SOK_T sok )
{
    struct msg *queued_msg;
    SIMPLE_MESSAGE_PTR s_mesg;
    DWORD type; 
    char *data;
    char *tmp;
    char *temp;
    char url_desc[1024], url_data[1024];
    net_icq_pak pak;

    if ((queued_msg = msg_queue_pop()) != NULL)
    {
        queued_msg->attempts++;
        if (queued_msg->attempts <= 6)
        {
            if ( Verbose )
            {
		R_undraw ();
                M_print( "\nResending message with SEQ num %04x.\tCMD ", (queued_msg->seq>>16) );
		Print_CMD( Chars_2_Word( &queued_msg->body[CMD_OFFSET] ) );
		M_print( "(Attempt #%d.)", queued_msg->attempts);
		M_print( "%d\n",queued_msg->len );
		R_redraw ();
            }
            if ( 0x1000 < Chars_2_Word( &queued_msg->body[CMD_OFFSET] ) ) {
		Dump_Queue();
            }
	    temp = malloc( queued_msg->len + 3 ); /* make sure packet fits in DWORDS */
	    assert( temp != NULL );
/*	    M_print( "Pre!\n" );*/
	    memcpy( temp, queued_msg->body, queued_msg->len);
/*	    Hex_Dump( temp, queued_msg->len );*/
            SOCKWRITE_LOW(sok, temp, queued_msg->len);
	    free(temp);
            queued_msg->exp_time = time(NULL) + 10; 
            msg_queue_push( queued_msg );
        }
        else
        {
            memcpy(&pak.head.ver, queued_msg->body, queued_msg->len); 
            R_undraw ();
	    if ( CMD_SENDM  == Chars_2_Word( pak.head.cmd ) ) {
               s_mesg = ( SIMPLE_MESSAGE_PTR ) pak.data;
               M_print("\nDiscarding message to ");
               Print_UIN_Name( Chars_2_DW( s_mesg->uin ) );
               M_print(" after %d send attempts.  Message content:\n",
                       queued_msg->attempts - 1);

               type = Chars_2_Word(s_mesg->type);
               data = s_mesg->len + 2; 
               if (type == URL_MESS || type == MRURL_MESS)
               {
                   tmp = strchr( data, '\xFE' );
                   if ( tmp != NULL )
                   {
                       *tmp = 0;
                       rus_conv("wk", data);
                       strcpy(url_desc, data);
                       tmp++;
                       data = tmp;
                       rus_conv("wk", data);
                       strcpy(url_data, data);

                       M_print( " Description: " MESSCOL "%s" \
                               NOCOL "\n", url_desc );
                       M_print( " URL        : " MESSCOL "%s" NOCOL " ", 
                               url_data );
                   }
               }
               else if (type == NORM_MESS || type == MRNORM_MESS)
               {
                   rus_conv("wk", data); 
                   M_print( MESSCOL "%s", data );
                   M_print( NOCOL " " );
               }
            } else {
	       M_print( "\nDiscarded a " );
	       Print_CMD( Chars_2_Word( pak.head.cmd ) );
	       M_print( " packet." );
	       if ( ( CMD_LOGIN == Chars_2_Word( pak.head.cmd ) ) ||
	            ( CMD_KEEP_ALIVE == Chars_2_Word( pak.head.cmd ) ) ) {
                  M_print( "\n\aConnection unstable exiting...." );
		  Quit = TRUE;
	       }
	    }
	    M_print ("\n");
	    R_redraw ();

            free(queued_msg->body);
            free(queued_msg);
        }

        if ( (queued_msg = msg_queue_peek() ) != NULL )
        {
            next_resend = queued_msg->exp_time; 
        }
        else
        {
            next_resend = INT_MAX;
        }
    }
    else
    {
        next_resend = INT_MAX;
    }
}

/*********************************
This must be called to remove messages
from the server
**********************************/
void snd_got_messages( int sok )
{
   net_icq_pak pak;
   
   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_ACK_MESSAGES );
   Word_2_Chars( pak.head.seq, seq_num++ );
   DW_2_Chars( pak.head.UIN, UIN );
   DW_2_Chars( pak.data, rand() );
   
   last_cmd[ (seq_num - 1) & 0x3ff ] = Chars_2_Word( pak.head.cmd );
   SOCKWRITE( sok, &(pak.head.ver), sizeof( pak.head ) - 2 + 4 );
}

/*************************************
this sends over the contact list
**************************************/
void snd_contact_list( int sok )
{
   net_icq_pak pak;
   int num_used;
   int i, size;
   int j;
   char *tmp;
   
   for ( i=0; i < Num_Contacts ;  )
   {
      Word_2_Chars( pak.head.ver, ICQ_VER );
      Word_2_Chars( pak.head.cmd, CMD_CONT_LIST );
      Word_2_Chars( pak.head.seq, seq_num++ );
      DW_2_Chars( pak.head.UIN, UIN );

      tmp = pak.data;
      tmp++;
      for ( j=0, num_used=0;  (j < MAX_CONTS_PACKET) && ( i < Num_Contacts );  i++ )
      {
	 if ( (S_DWORD) Contacts[ i ].uin >  0 )
	 {
            DW_2_Chars( tmp, Contacts[i].uin );
            tmp+=4;
            num_used++;
	    j++;
	 }
      }
      pak.data[0] = num_used;
      size = sizeof( DWORD ) * num_used + 1;
      size += sizeof( pak.head ) - 2;
      last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
      SOCKWRITE( sok, &(pak.head.ver), size );
/*      M_print( "Sent %d Contacts.\n", num_used );*/
   }
}

/*************************************
this sends over the Invisible list
that allows certain users to see you
if you're invisible.
**************************************/
void snd_invis_list( int sok )
{
   net_icq_pak pak;
   int num_used;
   int i, size;
   char *tmp;
   
   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_INVIS_LIST );
   Word_2_Chars( pak.head.seq, seq_num );
   DW_2_Chars( pak.head.UIN, UIN );
   
   tmp = pak.data;
   tmp++;
   for ( i=0, num_used=0; i < Num_Contacts ; i++ )
   {
      if ( (S_DWORD) Contacts[ i ].uin >  0 )
      {
         if ( Contacts[i].invis_list )
         {
            DW_2_Chars( tmp, Contacts[i].uin );
            tmp+=4;
            num_used++;
         }
      }
   }
   if ( num_used != 0 )
   {
      pak.data[0] = num_used;
      size = ( ( int ) tmp - ( int ) pak.data );
      size += sizeof( pak.head ) - 2;
      last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
      SOCKWRITE( sok, &(pak.head.ver), size );
      seq_num++;
   }
}

/*************************************
this sends over the Visible list
that allows certain users to see you
if you're invisible.
**************************************/
void snd_vis_list( int sok )
{
   net_icq_pak pak;
   int num_used;
   int i, size;
   char *tmp;
   
   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_VIS_LIST );
   Word_2_Chars( pak.head.seq, seq_num );
   DW_2_Chars( pak.head.UIN, UIN );
   
   tmp = pak.data;
   tmp++;
   for ( i=0, num_used=0; i < Num_Contacts ; i++ )
   {
      if ( (S_DWORD) Contacts[ i ].uin >  0 )
      {
         if ( Contacts[i].vis_list )
         {
            DW_2_Chars( tmp, Contacts[i].uin );
            tmp+=4;
            num_used++;
	    if ( Contacts[i].invis_list ) {
	       M_print( "ACK!!! %d\n",Contacts[ i ].uin ); 
	    }
         }
      }
   }
   if ( num_used != 0 )
   {
      pak.data[0] = num_used;
      size = ( ( int ) tmp - ( int ) pak.data );
      size += sizeof( pak.head ) - 2;
      last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
      SOCKWRITE( sok, &(pak.head.ver), size );
      seq_num++;
   }
}

/**************************************
This sends the second login command
this is necessary to finish logging in.
***************************************/
void snd_login_1( int sok )
{
   net_icq_pak pak;
   
   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_LOGIN_1 );
   Word_2_Chars( pak.head.seq, seq_num++ );
   DW_2_Chars( pak.head.UIN, UIN );
   DW_2_Chars( pak.data, rand() );
   
   last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
   SOCKWRITE( sok, &(pak.head.ver), sizeof( pak.head ) - 2 + 4);
}

/*********************************
This must be called every 2 min.
so the server knows we're still alive.
JAVA client sends two different commands
so we do also :)
**********************************/
void Keep_Alive( int sok )
{
   net_icq_pak pak;
   
   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_KEEP_ALIVE );
   Word_2_Chars( pak.head.seq, seq_num++ );
   DW_2_Chars( pak.head.UIN, UIN );
   DW_2_Chars( pak.data, rand() );
   
   last_cmd[(seq_num - 1) & 0x3ff ] = Chars_2_Word( pak.head.cmd );
   SOCKWRITE( sok, &(pak.head.ver), sizeof( pak.head ) - 2 + 4);

   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_KEEP_ALIVE2 );
   Word_2_Chars( pak.head.seq, seq_num++ );
   DW_2_Chars( pak.head.UIN, UIN );
   DW_2_Chars( pak.data, rand() );
   
   last_cmd[(seq_num - 1) & 0x3ff ] = Chars_2_Word( pak.head.cmd );
   SOCKWRITE( sok, &(pak.head.ver), sizeof( pak.head ) - 2 + 4);
   if ( Verbose )
   {
   R_undraw ();
#ifdef FUNNY_MSGS
   M_print( "\nIf you go to" MESSCOL " Z'Ha'Dum " NOCOL "you will die!!!!\n" ); 
   /* or if you don't send this packet */
#else
   M_print( "\nSend Keep_Alive packet to the server\n" );
#endif
   R_redraw ();
   }
}

/********************************************************
The following data constitutes fair use for compatibility.
*********************************************************/
static const BYTE table[] = {
 0x59, 0x60, 0x37 , 0x6B , 0x65 , 0x62 , 0x46 , 0x48 , 0x53 , 0x61 , 0x4C , 0x59 , 0x60 , 0x57 , 0x5B , 0x3D,
 0x5E, 0x34, 0x6D , 0x36 , 0x50 , 0x3F , 0x6F , 0x67 , 0x53 , 0x61 , 0x4C , 0x59 , 0x40 , 0x47 , 0x63 , 0x39,
 0x50, 0x5F, 0x5F , 0x3F , 0x6F , 0x47 , 0x43 , 0x69 , 0x48 , 0x33 , 0x31 , 0x64 , 0x35 , 0x5A , 0x4A , 0x42,
 0x56, 0x40, 0x67 , 0x53 , 0x41 , 0x07 , 0x6C , 0x49 , 0x58 , 0x3B , 0x4D , 0x46 , 0x68 , 0x43 , 0x69 , 0x48,
 0x33, 0x31, 0x44 , 0x65 , 0x62 , 0x46 , 0x48 , 0x53 , 0x41 , 0x07 , 0x6C , 0x69 , 0x48 , 0x33 , 0x51 , 0x54,
 0x5D, 0x4E, 0x6C , 0x49 , 0x38 , 0x4B , 0x55 , 0x4A , 0x62 , 0x46 , 0x48 , 0x33 , 0x51 , 0x34 , 0x6D , 0x36,
 0x50, 0x5F, 0x5F , 0x5F , 0x3F , 0x6F , 0x47 , 0x63 , 0x59 , 0x40 , 0x67 , 0x33 , 0x31 , 0x64 , 0x35 , 0x5A,
 0x6A, 0x52, 0x6E , 0x3C , 0x51 , 0x34 , 0x6D , 0x36 , 0x50 , 0x5F , 0x5F , 0x3F , 0x4F , 0x37 , 0x4B , 0x35,
 0x5A, 0x4A, 0x62 , 0x66 , 0x58 , 0x3B , 0x4D , 0x66 , 0x58 , 0x5B , 0x5D , 0x4E , 0x6C , 0x49 , 0x58 , 0x3B,
 0x4D, 0x66, 0x58 , 0x3B , 0x4D , 0x46 , 0x48 , 0x53 , 0x61 , 0x4C , 0x59 , 0x40 , 0x67 , 0x33 , 0x31 , 0x64,
 0x55, 0x6A, 0x32 , 0x3E , 0x44 , 0x45 , 0x52 , 0x6E , 0x3C , 0x31 , 0x64 , 0x55 , 0x6A , 0x52 , 0x4E , 0x6C,
 0x69, 0x48, 0x53 , 0x61 , 0x4C , 0x39 , 0x30 , 0x6F , 0x47 , 0x63 , 0x59 , 0x60 , 0x57 , 0x5B , 0x3D , 0x3E,
 0x64, 0x35, 0x3A , 0x3A , 0x5A , 0x6A , 0x52 , 0x4E , 0x6C , 0x69 , 0x48 , 0x53 , 0x61 , 0x6C , 0x49 , 0x58,
 0x3B, 0x4D, 0x46 , 0x68 , 0x63 , 0x39 , 0x50 , 0x5F , 0x5F , 0x3F , 0x6F , 0x67 , 0x53 , 0x41 , 0x25 , 0x41,
 0x3C, 0x51, 0x54 , 0x3D , 0x5E , 0x54 , 0x5D , 0x4E , 0x4C , 0x39 , 0x50 , 0x5F , 0x5F , 0x5F , 0x3F , 0x6F,
 0x47, 0x43, 0x69 , 0x48 , 0x33 , 0x51 , 0x54 , 0x5D , 0x6E , 0x3C , 0x31 , 0x64 , 0x35 , 0x5A , 0x00 , 0x00,
};

/***************************************************
Sends a message thru the server to uin.  Text is the
message to send.
****************************************************/
void icq_sendmsg( SOK_T sok, DWORD uin, char *text, DWORD msg_type)
{
	SIMPLE_MESSAGE msg;
	net_icq_pak pak;
	int size, len; 

        if ( UIN2nick( uin ) != NULL )
           log_event( LOG_MESS, "You sent instant message to %s\n%s\n", UIN2nick( uin ), text );
	else
           log_event( LOG_MESS, "You sent instant message to %d\n%s\n", uin, text );

	rus_conv ("kw",text);
	
	len = strlen(text);
	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_SENDM );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );

	DW_2_Chars( msg.uin, uin );
	Word_2_Chars(msg.type, msg_type);
	Word_2_Chars( msg.len, len + 1 );		/* length + the NULL */

	memcpy(&pak.data, &msg, sizeof( msg ) );
	memcpy(&pak.data[8], text, len + 1);

	size = sizeof( msg ) + len + 1;

	last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
}

/**************************************************
Sends a authorixation to the server so the Mirabilis
client can add the user.
***************************************************/
void icq_sendauthmsg( SOK_T sok, DWORD uin)
{
	SIMPLE_MESSAGE msg;
	net_icq_pak pak;
	int size; 

	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_SENDM );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );

	DW_2_Chars( msg.uin, uin );
	DW_2_Chars( msg.type, AUTH_MESSAGE );		/* A type authorization msg*/
	Word_2_Chars( msg.len, 2 );		

	memcpy(&pak.data, &msg, sizeof( msg ) );

   pak.data[ sizeof(msg) ]=0x03;
   pak.data[ sizeof(msg) + 1]=0x00;

	size = sizeof( msg ) + 2;

	last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
}

/***************************************************
Requests a random user from the specified group.
****************************************************/
void icq_rand_user_req( SOK_T sok, DWORD group )
{
   net_icq_pak pak;
   int size ;


   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_RAND_SEARCH );
   Word_2_Chars( pak.head.seq, seq_num++ );
   DW_2_Chars( pak.head.UIN, UIN );

   DW_2_Chars( pak.data, group);

   size = 4;

   last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
   SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
}

/***************************************************
Sets our Random chat group
****************************************************/
void icq_rand_set( SOK_T sok, DWORD group )
{
   net_icq_pak pak;
   int size ;


   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_RAND_SET );
   Word_2_Chars( pak.head.seq, seq_num++ );
   DW_2_Chars( pak.head.UIN, UIN );

   DW_2_Chars( pak.data, group);

   size = 4;

   last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
   SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
}


/***************************************************
Changes the users status on the server
****************************************************/
void icq_change_status( SOK_T sok, DWORD status )
{
	net_icq_pak pak;
	int size ;


	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_STATUS_CHANGE );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );

   DW_2_Chars( pak.data, status);
   Current_Status = status;

	size = 4;

	last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
}

/******************************************************************
Logs off ICQ should handle other cleanup as well
********************************************************************/
void Quit_ICQ( int sok )
{
	net_icq_pak pak;
	int size, len;
   
	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_SEND_TEXT_CODE );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );
   
   len = strlen( "B_USER_DISCONNECTED" ) + 1;
   *(short * ) pak.data = len;
   size = len + 4;
   
   memcpy( &pak.data[2], "B_USER_DISCONNECTED", len );
   pak.data[ 2 + len ] = 05;
   pak.data[ 3 + len ] = 00;

   last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
   SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);

   SOCKCLOSE(sok);
   SOCKCLOSE( sok );
}

void info_req_99( SOK_T sok, DWORD uin )
{
	net_icq_pak pak;
	int size ;

	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_META_USER );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );
#if ICQ_VER == 0x0002
   Word_2_Chars( pak.data , seq_num );
   DW_2_Chars( pak.data + 2, uin );

	size = 6;
#else
   Word_2_Chars( pak.data , META_INFO_REQ );
   DW_2_Chars( pak.data + 2, uin );

	size = 6;
#endif

	last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);

}

/*********************************************************
Sends a request to the server for info on a specific user
**********************************************************/
void send_info_req( SOK_T sok, DWORD uin )
{
	net_icq_pak pak;
	int size ;

#if 1
	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_INFO_REQ );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );
#if ICQ_VER == 0x0002
   Word_2_Chars( pak.data , seq_num );
   DW_2_Chars( pak.data + 2, uin );

	size = 6;
#else
   DW_2_Chars( pak.data, uin );

	size = 4;
#endif

	last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
#endif
}

/*********************************************************
Sends a request to the server for info on a specific user
**********************************************************/
void send_ext_info_req( SOK_T sok, DWORD uin )
{
	net_icq_pak pak;
	int size ;


	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_EXT_INFO_REQ );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );

#if ICQ_VER == 0x0002
   Word_2_Chars( pak.data , seq_num );
   DW_2_Chars( pak.data + 2, uin );
	size = 6;
#else
   DW_2_Chars( pak.data , uin );
	size = 4;
#endif


	last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
}

/***************************************************************
Initializes a server search for the information specified
****************************************************************/
void start_search( SOK_T sok, char *email, char *nick, char* first, char* last )
{
	net_icq_pak pak;
	int size ;

	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_SEARCH_USER );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );
/*
   Word_2_Chars( pak.data , seq_num++ );
   size = 2;
*/
   size = 0;
   Word_2_Chars( pak.data + size, strlen( nick ) + 1 );
   size += 2;
   strcpy( (char *) (pak.data+size) , nick );
   size += strlen( nick ) + 1;
   Word_2_Chars( pak.data + size, strlen( first ) + 1 );
   size += 2;
   strcpy( (char *) (pak.data+size) , first );
   size += strlen( first ) + 1;
   Word_2_Chars( pak.data + size, strlen( last ) + 1);
   size += 2;
   strcpy( (char *) (pak.data+size) , last );
   size += strlen( last ) +1 ;
   Word_2_Chars( pak.data + size, strlen( email ) + 1  );
   size += 2;
   strcpy( (char *) (pak.data+size) , email );
   size += strlen( email ) + 1;
	last_cmd[seq_num - 2 ] = Chars_2_Word( pak.head.cmd );
	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
}

/***************************************************
Registers a new UIN in the ICQ network
****************************************************/
void reg_new_user( SOK_T sok, char *pass)
{
#if ICQ_VER == 0x0002
	srv_net_icq_pak pak;
#else
	net_icq_pak pak;
#endif
   char len_buf[2];
	int size, len; 

	len = strlen(pass);
	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_REG_NEW_USER );
	Word_2_Chars( pak.head.seq, seq_num++ );
#if ICQ_VER != 0x0002
	Word_2_Chars( pak.head.seq2, seq_num-1 );
#endif
   Word_2_Chars( len_buf, len );
#if ICQ_VER == 0x0002
	memcpy(&pak.data, "\x02\x00", 2 );
   memcpy(&pak.data[2], len_buf, 2 );
	memcpy(&pak.data[4], pass, len + 1);
   DW_2_Chars( &pak.data[4+len], 0x0072 );
   DW_2_Chars( &pak.data[8+len], 0x0000 );
 	size = len + 12;
#else
/*	memcpy(&pak.data, "\x02\x00", 2 );*/
   memcpy(&pak.data[0], len_buf, 2 );
	memcpy(&pak.data[2], pass, len + 1);
   DW_2_Chars( &pak.data[2+len], 0xA0 );
   DW_2_Chars( &pak.data[6+len], 0x2461 );
   DW_2_Chars( &pak.data[10+len], 0xa00000 );
   DW_2_Chars( &pak.data[14+len], 0x00 );
	size = len + 18;
#endif
#if ICQ_VER == 0x0005
   DW_2_Chars( pak.head.session, rand() );
   DW_2_Chars( pak.head.zero, 0L );
   DW_2_Chars( pak.head.UIN, 0L );
#endif

	last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
	SOCKWRITE_LOW( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
}

/******************************************************
Changes the password on the server.
*******************************************************/
void Change_Password( SOK_T sok, char *pass)
{
#if ICQ_VER == 0x0002
	srv_net_icq_pak pak;
#else
	net_icq_pak pak;
#endif
   char len_buf[2];
	int size, len; 

	len = strlen(pass);
	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_META_USER );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );
#if ICQ_VER != 0x0002
	Word_2_Chars( pak.head.seq2, seq_num-1 );
#endif
   len++;
   Word_2_Chars( len_buf, len );
   Word_2_Chars( pak.data, META_INFO_PASS );
   memcpy(&pak.data[2], len_buf, 2 );
	memcpy(&pak.data[4], pass, len);
 	size = len + 4;

	last_cmd[seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
}

void Update_User_Info( SOK_T sok, USER_INFO_PTR user)
{
	net_icq_pak pak;
	int size ;

	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_UPDATE_INFO );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );
   
#if ICQ_VER == 0x0002
   Word_2_Chars( pak.data , seq_num++ );
   size = 2;
#else
   size = 0;
#endif
   Word_2_Chars( pak.data + size, strlen( user->nick ) + 1 );
   size += 2;
   strcpy( (char *) (pak.data+size) , user->nick );
   size += strlen( user->nick ) + 1;
   Word_2_Chars( pak.data + size, strlen( user->first ) + 1 );
   size += 2;
   strcpy( (char *) (pak.data+size) , user->first );
   size += strlen( user->first ) + 1;
   Word_2_Chars( pak.data + size, strlen( user->last ) + 1);
   size += 2;
   strcpy( (char *) (pak.data+size) , user->last );
   size += strlen( user->last ) +1 ;
   Word_2_Chars( pak.data + size, strlen( user->email ) + 1  );
   size += 2;
   strcpy( (char *) (pak.data+size) , user->email );
   size += strlen( user->email ) + 1;
   pak.data[ size ] = user->auth;
   size++;
	last_cmd[ ( seq_num - 1 ) & 0x3ff ] = Chars_2_Word( pak.head.cmd );
	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);

	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_AUTH_UPDATE );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );

   DW_2_Chars( pak.data, ! user->auth );
	last_cmd[ ( seq_num - 1 ) & 0x3ff ] = Chars_2_Word( pak.head.cmd );
	SOCKWRITE( sok, &(pak.head.ver), 4 + sizeof( pak.head ) - 2);
}

void Update_More_User_Info( SOK_T sok, MORE_INFO_PTR user)
{
	net_icq_pak pak;
	int size ;

	Word_2_Chars( pak.head.ver, ICQ_VER );
	Word_2_Chars( pak.head.cmd, CMD_META_USER );
	Word_2_Chars( pak.head.seq, seq_num++ );
	DW_2_Chars( pak.head.UIN, UIN );
   
   Word_2_Chars( pak.data , META_INFO_SET );
   size = 2;
   Word_2_Chars( pak.data + size, strlen( user->nick ) + 1 );
   size += 2;
   strcpy( (char *) (pak.data+size) , user->nick );
   size += strlen( user->nick ) + 1;
   Word_2_Chars( pak.data + size, strlen( user->first ) + 1 );
   size += 2;
   strcpy( (char *) (pak.data+size) , user->first );
   size += strlen( user->first ) + 1;
   Word_2_Chars( pak.data + size, strlen( user->last ) + 1);
   size += 2;
   strcpy( (char *) (pak.data+size) , user->last );
   size += strlen( user->last ) +1 ;
   Word_2_Chars( pak.data + size, strlen( user->email ) + 1  );
   size += 2;
   strcpy( (char *) (pak.data+size) , user->email );
   size += strlen( user->email ) + 1;
   Word_2_Chars( pak.data + size, strlen( user->email2 ) + 1  );
   size += 2;
   strcpy( (char *) (pak.data+size) , user->email2 );
   size += strlen( user->email2 ) + 1;
   Word_2_Chars( pak.data + size, strlen( user->email3 ) + 1  );
   size += 2;
   strcpy( (char *) (pak.data+size) , user->email3 );
   size += strlen( user->email3 ) + 1;
   Word_2_Chars( pak.data + size, strlen( user->city ) + 1  );
   size += 2;
   strcpy( (char *) (pak.data+size) , user->city );
   size += strlen( user->city ) + 1;
   Word_2_Chars( pak.data + size, strlen( user->state ) + 1  );
   size += 2;
   strcpy( (char *) (pak.data+size) , user->state );
   size += strlen( user->state ) + 1;
   Word_2_Chars( pak.data + size, strlen( user->phone ) + 1  );
   size += 2;
   strcpy( (char *) (pak.data+size) , user->phone );
   size += strlen( user->phone ) + 1;
   Word_2_Chars( pak.data + size, strlen( user->fax ) + 1  );
   size += 2;
   strcpy( (char *) (pak.data+size) , user->fax );
   size += strlen( user->fax ) + 1;
   Word_2_Chars( pak.data + size, strlen( user->street ) + 1  );
   size += 2;
   strcpy( (char *) (pak.data+size) , user->street );
   size += strlen( user->street ) + 1;
   Word_2_Chars( pak.data + size, strlen( user->cellular ) + 1  );
   size += 2;
   strcpy( (char *) (pak.data+size) , user->cellular );
   size += strlen( user->cellular ) + 1;
   DW_2_Chars( &pak.data[ size ] , user->zip );
   size += 4;
   Word_2_Chars( &pak.data[ size ] , user->country );
   size += 2;
   pak.data[ size ++ ] = user->c_status;
   pak.data[ size ] = user->hide_email;
   size++;
	last_cmd[ ( seq_num - 1 ) & 0x3ff ] = Chars_2_Word( pak.head.cmd );
	SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head ) - 2);
}

void icq_sendurl( SOK_T sok, DWORD uin, char *description, char *url )
{
   char buf[450];
   
   sprintf( buf, "%s\xFE%s", url, description );
   icq_sendmsg( sok, uin, buf, URL_MESS );
}

void Gen_Checksum( BYTE * buf, DWORD len )
{
   DWORD cc, cc2;
   DWORD r1,r2;
   
   cc = buf[8];
   cc <<= 8;
   cc += buf[4];
   cc <<= 8;
   cc += buf[2];
   cc <<= 8;
   cc += buf[6];

   r1 = rand() % ( len - 0x18 );
   r1 += 0x18;
   r2 = rand() & 0xff;
   
   cc2 = r1;
   cc2 <<= 8;
   cc2 += buf[ r1 ];
   cc2 <<= 8;
   cc2 += r2;   
   cc2 <<= 8;
   cc2 += table[ r2 ];
   cc2 ^= 0xff00ff;
/*   printf( "tbl[%02lX] = %02X\n%08lX\n\n", r2,table[ r2 ], cc ^ cc2 ); */
   
   DW_2_Chars( &buf[0x14], cc ^ cc2 );
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

void Wrinkle( void * buf, size_t len )
{
/*   DWORD plen=size;*/
   DWORD checkcode;
   DWORD code1, code2, code3;
   DWORD pos;
   DWORD data;
   
   Gen_Checksum( buf, len );
   checkcode = Chars_2_DW( &((BYTE*)buf)[ 20 ] );
   code1 = len * 0x68656c6cL;
   code2 = code1 + checkcode;
   pos = 0xa;
   for ( ; pos < ( len  ); pos+=4 )
   {
      code3 = code2 + table[ pos & 0xFF ];
      data = Chars_2_DW( &((BYTE*)buf)[ pos ] );
      data ^= code3;
      DW_2_Chars( &((BYTE*)buf)[ pos ], data );
   }
   checkcode = Scramble_cc( checkcode );
   DW_2_Chars( &((BYTE*)buf)[ 0x14 ], checkcode );
}

/***************************************************************
This handles actually sending a packet after it's been assembled.
When V5 is implemented this will wrinkle the packet and calculate 
the checkcode.
Adds packet to the queued messages.
****************************************************************/
size_t SOCKWRITE( SOK_T sok, void * ptr, size_t len )
{
   struct msg *msg_to_queue;
   static WORD seq2=0;
   WORD temp;
   WORD cmd;

#if ICQ_VER == 0x0004
   Word_2_Chars( &((BYTE *)ptr)[4], 0 );
   ((BYTE *)ptr)[0x0A] = ((BYTE *) ptr)[8];
   ((BYTE *)ptr)[0x0B] = ((BYTE *) ptr)[9];
#elif ICQ_VER == 0x0005
    DW_2_Chars( &((BYTE *)ptr)[2], 0L );
    if ( 0 == our_session ) {
       our_session = rand() & 0x3fffffff;
    }
    DW_2_Chars( &((BYTE *)ptr)[0x0A], our_session );
#endif
   cmd = Chars_2_Word( (((ICQ_PAK_PTR)((BYTE*)ptr-2))->cmd ) );
   if ( cmd != CMD_ACK ) {
      msg_to_queue = (struct msg *) malloc(sizeof(struct msg));
#if ICQ_VER == 0x0004
      msg_to_queue->seq = Chars_2_Word( &((BYTE *)ptr)[SEQ_OFFSET] );
#elif ICQ_VER == 0x0005
    if ( 0 == seq2 ) {
       seq2 = rand() & 0x7fff;
    }
    temp = Chars_2_Word( &((BYTE *)ptr)[SEQ2_OFFSET] );
    temp += seq2;
    Word_2_Chars( &((BYTE *)ptr)[SEQ_OFFSET], temp );
    if ( CMD_KEEP_ALIVE == Chars_2_Word( &((BYTE *)ptr)[CMD_OFFSET] ) ) {
       Word_2_Chars( &((BYTE *)ptr)[SEQ2_OFFSET], 0 );
/*       seq2++;*/
       seq_num--;
    }
      msg_to_queue->seq = Chars_2_DW( &((BYTE *)ptr)[SEQ_OFFSET] );
#else
#error Incorrect ICQ_VERsion
#endif
      msg_to_queue->attempts = 1;
      msg_to_queue->exp_time = time(NULL) + 10;
      msg_to_queue->body = (BYTE *) malloc( len );
      msg_to_queue->len = len;
      memcpy(msg_to_queue->body, ptr, msg_to_queue->len);
      msg_queue_push( msg_to_queue );

      if (msg_queue_peek() == msg_to_queue)
      {
	  next_resend = msg_to_queue->exp_time;
      }
   }

   return SOCKWRITE_LOW( sok, ptr, len );
}

/***************************************************************
This handles actually sending a packet after it's been assembled.
When V5 is implemented this will wrinkle the packet and calculate 
the checkcode.
Doesn't add packet to the queue.
****************************************************************/
static size_t SOCKWRITE_LOW( SOK_T sok, void * ptr, size_t len )
{
assert( len > 0x18 );
#if 1
      if ( Verbose > 1 ) {
         M_print( "\n" );
         Hex_Dump( ptr, len );
         M_print( "\n" );
      }
#endif 
   Wrinkle( ptr, len );
#if 0
      if ( Verbose > 1 )
         Hex_Dump( ptr, len );
#endif 
/*   Dump_Queue()*/
   return sockwrite( sok, ptr, len );
}

size_t SOCKREAD( SOK_T sok, void * ptr, size_t len )
{
   size_t sz;
   
   sz = sockread( sok, ptr, len );
   if ( ( Verbose > 2 ) && ( sz > 0 ) ) {
      M_print( "\n" );
      Hex_Dump( ptr, sz );
   }
   return sz;
}
