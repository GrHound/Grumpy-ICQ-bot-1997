#include "datatype.h"
#include "micq.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#ifdef _WIN32
  #include <winsock2.h>
#else
  #include <netinet/in.h>
  #include <unistd.h>
  #include <netdb.h>
#endif
#include <time.h>
#include <string.h>


void Meta_User( SOK_T sok, BYTE *data, DWORD len, DWORD uin )
{
   WORD subcmd;
   
   subcmd = Chars_2_Word( data );
   
   switch ( subcmd ) {
      case META_SRV_PASS:
         M_print( "\nPassword Change was " CLIENTCOL "%s" NOCOL ".", data[2] == 0xA ? "successful" : "unsuccessful" );
      break;
      default:
         M_print( "\nUnknown Meta User response " SERVCOL "%04X" NOCOL " ", subcmd );
	 if ( Verbose ) {
	    M_print( "\n" );
	    Hex_Dump( data, len );
	 }
      break;
   }
}

void Display_Rand_User( SOK_T sok, BYTE *data, DWORD len )
{
   if ( len == 37 ) {
      M_print( "Random User :\t%d\n", Chars_2_DW( &data[0] ) );
      M_print( "IP          :\t%d.%d.%d.%d\n", data[4], data[5], data[6], data[7]  );
      M_print( "Port        :\t%d\n", Chars_2_DW( &data[8] ) );
      M_print( "IP2         :\t%d.%d.%d.%d\n", data[12], data[13], data[14], data[15]  );
      M_print( "Connection  :\t%s\n", data[16] == 4 ? "Peer-to-Peer" : "Server Only" );
      M_print( "Status      :\t" );
      Print_Status( Chars_2_DW( &data[17] ) );
      M_print( "\nTCP version :\t%d", Chars_2_Word( &data[21] ) );
      if ( Verbose > 1 ) {
         printf( "\n" );
	 Hex_Dump( data, len );
      }
      send_info_req( sok, Chars_2_DW( data ) );
      send_ext_info_req( sok, Chars_2_DW( data ) );
      Last_Probed_Rand_User.uin = Chars_2_DW( &data[0] );
      Last_Probed_Rand_User.ready = 0;
      
   } else {
      M_print( "No Random User Found" );
   }
}

void Recv_Message( int sok, BYTE * pak )
{
   RECV_MESSAGE_PTR r_mesg;

/*   M_print( "\n" );*/
   r_mesg = ( RECV_MESSAGE_PTR )pak;
   last_recv_uin = Chars_2_DW( r_mesg->uin );
   Print_UIN_Name( Chars_2_DW( r_mesg->uin ) );
   M_print( ":\a\nDate %d/%d/%d\t%d:%02d GMT\n", r_mesg->month, r_mesg->day, 
            Chars_2_Word( r_mesg->year ), r_mesg->hour ,  r_mesg->minute );
            
   M_print( "Type : %d \t Len : %d\n", Chars_2_Word( r_mesg->type ),
       Chars_2_Word( r_mesg->len ) );
   Do_Msg( sok, Chars_2_Word( r_mesg->type )
              , Chars_2_Word( r_mesg->len )
              , (char *) ( r_mesg->len + 2 )
              , last_recv_uin ); 
   
/*   M_print( MESSCOL "%s\n" NOCOL, ((char *) &r_mesg->len) + 2 );*/
/*   ack_srv( sok, Chars_2_Word( pak.head.seq ) ); */
}


/************************************************
This is called when a user goes offline
*************************************************/
void User_Offline( int sok, BYTE * pak )
{
   int remote_uin;
   int index;

   remote_uin = Chars_2_DW( &pak[0] );

/*   M_print( "\n" );*/
   M_print( CONTACTCOL );
   index = Print_UIN_Name( remote_uin );
   M_print( NOCOL );
   M_print( " logged off.\t" );
   Time_Stamp();
/*   M_print( "\n" );*/
   if ( UIN2nick( remote_uin ) != NULL )
      log_event( LOG_ONLINE, "User logged off %s\n", UIN2nick( remote_uin ) );
   else
      log_event( LOG_ONLINE, "User logged off %d\n", remote_uin );
   if ( index != -1 )
   {
      Contacts[ index ].status = STATUS_OFFLINE;
      Contacts[ index ].last_time = time( NULL );
   }
}

void User_Online( int sok, BYTE * pak )
{
   int remote_uin, new_status;
   int index;

   remote_uin = Chars_2_DW( &pak[0] );

   new_status = Chars_2_DW( &pak[17] );
   
   if ( Done_Login )
   {
/*      M_print( "\n" );*/
      M_print( CONTACTCOL );
      index = Print_UIN_Name( remote_uin );
      M_print( NOCOL );
      if ( index != -1 )
      {
         Contacts[ index ].status = new_status;
         Contacts[ index ].current_ip[0] =  pak[4];
         Contacts[ index ].current_ip[1] =  pak[5];
         Contacts[ index ].current_ip[2] =  pak[6];
         Contacts[ index ].current_ip[3] =  pak[7];
         Contacts[ index ].port = Chars_2_DW( &pak[8] );
         Contacts[ index ].last_time = time( NULL );
      }
      M_print( " (" );
      Print_Status( new_status );
      M_print( ") logged on.\t" );
      Time_Stamp();
/*      M_print( "\n" );*/

      if ( UIN2nick( remote_uin ) != NULL )
         log_event( LOG_ONLINE, "User logged on %s\n", UIN2nick( remote_uin ) );
      else
         log_event( LOG_ONLINE, "User logged on %d\n", remote_uin );

      if ( Verbose )
      {
         M_print( "The IP address is %u.%u.%u.%u\n", pak[4], pak[5], pak[6], pak[7] );
         M_print( "The \"real\" IP address is %u.%u.%u.%u\n", pak[12], pak[13], pak[14], pak[15] );
	 M_print( "%s\n", pak[16] == 4 ? "Peer-to-Peer mode" : "Server Only Communication." );
	 M_print( "TCP ICQ version : %d\n", Chars_2_Word( &pak[21] ) );
	 Hex_Dump( pak, 0x2B );
      }
   }
   else
   {
      Kill_Prompt();
      for ( index=0; index < Num_Contacts; index++ )
      {
         if ( Contacts[index].uin == remote_uin )
         {
            Contacts[ index ].status = new_status;
            Contacts[ index ].current_ip[0] =  pak[4];
            Contacts[ index ].current_ip[1] =  pak[5];
            Contacts[ index ].current_ip[2] =  pak[6];
            Contacts[ index ].current_ip[3] =  pak[7];
            Contacts[ index ].port = Chars_2_DW( &pak[8] );
            Contacts[ index ].last_time = time( NULL );
            break;
         }
      }
   }
}

void Status_Update( int sok, BYTE * pak )
{
   CONTACT_PTR bud;
   int remote_uin, new_status;
   int index;

   remote_uin = Chars_2_DW( &pak[0] );

   new_status = Chars_2_DW( &pak[4] );
   bud = UIN2Contact( remote_uin );
   if ( bud != NULL ) {
      if ( bud->status == new_status ) {
          Kill_Prompt();
          return;
      }
   }
/*   M_print( "\n" );*/
   M_print( CONTACTCOL );
   index = Print_UIN_Name( remote_uin );
   M_print( NOCOL );
   if ( index != -1 )
   {
      Contacts[ index ].status = new_status;
   }
   M_print( " changed status to " );
   Print_Status( new_status );
   M_print( "\t" );
   Time_Stamp();
/*   M_print( "\n" );*/
   
}

/* This procedure logins into the server with UIN and pass
   on the socket sok and gives our ip and port.
   It does NOT wait for any kind of a response.         */
void Login( int sok, int UIN, char *pass, int ip, int port, DWORD status )
{
   net_icq_pak pak;
   int size;
   login_1 s1;
   login_2 s2;
	struct sockaddr_in sin;  /* used to store inet addr stuff  */
   
   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_LOGIN );
   Word_2_Chars( pak.head.seq, seq_num++ );
   DW_2_Chars( pak.head.UIN, UIN );
   
   DW_2_Chars( s1.port, ntohs( port ) + 0x10000 );
   Word_2_Chars( s1.len, strlen( pass ) + 1 );
   DW_2_Chars( s1.time, time( NULL ) );  
   
   DW_2_Chars( s2.ip, ip );
   sin.sin_addr.s_addr = Chars_2_DW( s2.ip );
   DW_2_Chars( s2.status, status );
/*   Word_2_Chars( s2.seq, seq_num++ );*/
   
   DW_2_Chars( s2.X1, LOGIN_X1_DEF );
   s2.X2[0] = LOGIN_X2_DEF;
   DW_2_Chars( s2.X3, LOGIN_X3_DEF );
   DW_2_Chars( s2.X4, LOGIN_X4_DEF );
   DW_2_Chars( s2.X5, LOGIN_X5_DEF );
   
   memcpy( pak.data, &s1, sizeof( s1 ) );
   size = sizeof( s1 );
   memcpy( &pak.data[size], pass, Chars_2_Word( s1.len ) );
   size += Chars_2_Word( s1.len );
   memcpy( &pak.data[size], &s2.X1, sizeof( s2.X1 ) );
   size += sizeof( s2.X1 );
   memcpy( &pak.data[size], &s2.ip, sizeof( s2.ip ) );
   size += sizeof( s2.ip );
   memcpy( &pak.data[size], &s2.X2, sizeof( s2.X2 ) );
   size += sizeof( s2.X2 );
   memcpy( &pak.data[size], &s2.status, sizeof( s2.status ) );
   size += sizeof( s2.status );
   memcpy( &pak.data[size], &s2.X3, sizeof( s2.X3 ) );
   size += sizeof( s2.X3 );
/*   memcpy( &pak.data[size], &s2.seq, sizeof( s2.seq ) );
/   size += sizeof( s2.seq );*/
   memcpy( &pak.data[size], &s2.X4, sizeof( s2.X4 ) );
   size += sizeof( s2.X4 );
   memcpy( &pak.data[size], &s2.X5, sizeof( s2.X5 ) );
   size += sizeof( s2.X5 );
#if ICQ_VER == 0x0004
   last_cmd[ seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
#elif ICQ_VER == 0x0005
   last_cmd[ seq_num - 1 ] = Chars_2_Word( pak.head.cmd );
#else
   last_cmd[ seq_num - 2 ] = Chars_2_Word( pak.head.cmd );
#endif
   SOCKWRITE( sok, &(pak.head.ver), size + sizeof( pak.head )- 2 );
} 

/* This routine sends the aknowlegement cmd to the
   server it appears that this must be done after
   everything the server sends us                 */
void ack_srv( SOK_T sok, DWORD seq )
{
   net_icq_pak pak;
   
   Word_2_Chars( pak.head.ver, ICQ_VER );
   Word_2_Chars( pak.head.cmd, CMD_ACK );
   DW_2_Chars( pak.head.seq2, seq );
   DW_2_Chars( pak.head.UIN, UIN);
   DW_2_Chars( pak.data, rand() );
   
   SOCKWRITE( sok, &(pak.head.ver), sizeof( pak.head ) - 2 + 4 );
/*   Hex_Dump( &(pak.head.ver), sizeof( pak.head ) - 2 + 4 );*/

}

void Display_Info_Reply( int sok, BYTE * pak )
{
   char *tmp;
   int len;
   long iuin;
   
   iuin = Chars_2_DW( &pak[0] );
   
   M_print( SERVCOL "Info for %ld\n", iuin );
   len = Chars_2_Word( &pak[4] );
   rus_conv( "wk", (char *) &pak[6] );
   M_print( "Nick Name :\t%s\n", &pak[6] );
   Last_Probed_Rand_User.uin = iuin;
   strcpy(Last_Probed_Rand_User.nickname, (char *) &pak[6]);
  
   tmp = (char *) &pak[6 + len ];
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   M_print( "First name :\t%s\n", tmp+2 );
   strcpy(Last_Probed_Rand_User.firstname, tmp+2);
   Last_Probed_Rand_User.ready = 1;
   
   tmp += len + 2;
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   M_print( "Last name :\t%s\n", tmp+2 );
   
   tmp += len + 2;
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   M_print( "Email Address :\t%s\n", tmp+2 );
   
   tmp += len + 2;
   if ( *tmp == 1 )
   {
      M_print( "No authorization needed." NOCOL " " );
   }
   else
   {
      M_print( "Must request authorization." NOCOL " " );
   }
/*   ack_srv( sok, Chars_2_Word( pak.head.seq ) ); */
}

void Display_Ext_Info_Reply( int sok, BYTE * pak )
{
   unsigned char *tmp;
   int len;

   M_print( SERVCOL "More Info for %ld\n", Chars_2_DW( &pak[0] ) );
   len = Chars_2_Word( &pak[4] );
   rus_conv( "wk", &pak[6] );
   M_print( "City         :\t%s\n", &pak[6] );
   if ( Get_Country_Name( Chars_2_Word(&pak[6+len]) ) != NULL )
      M_print( "Country      :\t%s\n",Get_Country_Name( Chars_2_Word(&pak[6+len]) ) );
   else
      M_print( "Country Code :\t%d\n", Chars_2_Word( &pak[6+len] ) );
   M_print( "Time Zone    :\tGMT %+d\n", ((signed char) pak[len+8])>>1  );
   tmp = &pak[9 + len ];
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   M_print( "State        :\t%s\n", tmp+2 );
   if ( Chars_2_Word( tmp+2+len ) != 0xffff )
      M_print( "Age          :\t%d\n", Chars_2_Word( tmp+2+len ) );
   else
      M_print( "Age          :\tNot Entered\n");
   if (*(tmp + len + 4) == 2 )
      M_print( "Sex          :\tMale\n" );
   else if (*(tmp + len + 4) == 1 )
      M_print( "Sex          :\tFemale\n" );
   else
#ifdef FUNNY_MSGS
      M_print( "Sex          :\tYes please!\n" );
#else
      M_print( "Sex          :\tNot specified\n" );
#endif
   tmp += len + 5;
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   M_print( "Phone Number :\t%s\n", tmp+2 );
   tmp += len + 2;
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   M_print( "Home Page    :\t%s\n", tmp+2 );
   tmp += len + 2;
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   M_print( "About        :\n%s", tmp+2 );
/*   ack_srv( sok, Chars_2_Word( pak.head.seq ) ); */
}

void Display_Search_Reply( int sok, BYTE * pak )
{
   char *tmp;
   int len;
   M_print( SERVCOL "User found %ld\n", Chars_2_DW( &pak[0] ) );
   len = Chars_2_Word( &pak[4] );
   rus_conv( "wk", &pak[6] );
   M_print( "Nick Name :\t%s\n", &pak[6] );
   tmp = &pak[6 + len ];
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   M_print( "First name :\t%s\n", tmp+2 );
   tmp += len + 2;
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   M_print( "Last name :\t%s\n", tmp+2 );
   tmp += len + 2;
   len = Chars_2_Word( tmp );
   rus_conv( "wk", tmp+2 );
   M_print( "Email Address :\t%s\n", tmp+2 );
   tmp += len + 2;
   if ( *tmp == 1 )
   {
      M_print( "No authorization needed." NOCOL " " );
   }
   else
   {
      M_print( "Must request authorization." NOCOL " " );
   }
}

void Do_Msg( SOK_T sok, DWORD type, WORD len, char * data, DWORD uin )
{
   char *tmp;
	int   x,m;
   char message[1024];
   char url_data[1024];
   char url_desc[1024];

   if ( type == USER_ADDED_MESS )
   {
      tmp = strchr( data, '\xFE' );
      if ( tmp == NULL )
      {
         M_print( "Ack!!!!!!!  Bad packet" );
         return;
      }
      *tmp = 0;
      M_print( CONTACTCOL "\n%s" NOCOL " has added you to their contact list.\n", data );
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\xFE' );
      if ( tmp == NULL )
      {
         M_print( "Ack!!!!!!!  Bad packet" );
         return;
      }
      *tmp = 0;
      rus_conv ("wk",data);
      M_print( "First name    : " MESSCOL "%s" NOCOL "\n" , data );
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\xFE' );
      if ( tmp == NULL )
      {
         M_print( "Ack!!!!!!!  Bad packet" );
         return;
      }
      *tmp = 0;
      rus_conv ("wk",data);
      M_print( "Last name     : " MESSCOL "%s" NOCOL "\n" , data );
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\xFE' );
      *tmp = 0;
      rus_conv ("wk",data);
      M_print( "Email address : " MESSCOL "%s" NOCOL "\n" , data );
   }
   else if ( type == AUTH_REQ_MESS )
   {
      tmp = strchr( data, '\xFE' );
      *tmp = 0;
      M_print( CONTACTCOL "\n%s" NOCOL " has requested your authorization to be added to their contact list.\n", data );
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\xFE' );
      if ( tmp == NULL )
      {
         M_print( "Ack!!!!!!!  Bad packet" );
         return;
      }
      *tmp = 0;
      rus_conv ("wk",data);
      M_print( "First name    : " MESSCOL "%s" NOCOL "\n" , data );
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\xFE' );
      if ( tmp == NULL )
      {
         M_print( "Ack!!!!!!!  Bad packet" );
         return;
      }
      *tmp = 0;
      rus_conv ("wk",data);
      M_print( "Last name     : " MESSCOL "%s" NOCOL "\n" , data );
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\xFE' );
      if ( tmp == NULL )
      {
         M_print( "Ack!!!!!!!  Bad packet" );
         return;
      }
      *tmp = 0;
      rus_conv ("wk",data);
      M_print( "Email address : " MESSCOL "%s" NOCOL "\n" , data );
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\xFE' );
      if ( tmp == NULL )
      {
         M_print( "Ack!!!!!!!  Bad packet" );
         return;
      }
      *tmp = 0;
      tmp++;
      data = tmp;
      tmp = strchr( tmp, '\x00' );
      if ( tmp == NULL )
      {
         M_print( "Ack!!!!!!!  Bad packet" );
         return;
      }
      *tmp = 0;
      rus_conv ("wk",data);
      M_print( "Reason : " MESSCOL "%s" NOCOL "\n" , data );
   }
   else if (type == URL_MESS || type == MRURL_MESS)
   {

      tmp = strchr( data, '\xFE' );
      if ( tmp == NULL )
      {
         M_print( "Ack!!!!!!!  Bad packet" );
         return;
      }
      *tmp = 0;
      rus_conv ("wk",data);
      strcpy (url_desc,data);
      tmp++;
      data = tmp;
      rus_conv ("wk",data);
      strcpy (url_data,data);
      
      sprintf (message,"Description: %s \n                          URL: %s",url_desc,url_data);  
      if ( UIN2nick( uin ) != NULL )
         log_event( LOG_MESS, "You received URL message from %s\n%s\n", UIN2nick(uin), message );
      else
         log_event( LOG_MESS, "You received URL message from %d\n%s\n", uin, message );

      M_print( " URL Message.\n Description: " MESSCOL "%s" NOCOL "\n", url_desc );
      M_print(               " URL        : " MESSCOL "%s" NOCOL "\n", url_data );
   }
	else if (type == CONTACT_MESS || type==MRCONTACT_MESS)
	{
      tmp = strchr( data, '\xFE' );
      *tmp = 0;
      M_print( "\nContact List.\n" MESSCOL "============================================\n" NOCOL "%d Contacts\n", atoi(data) );
      tmp++;
      m = atoi(data);
      for(x=0; m > x ; x++)
      {
         data = tmp;
         tmp = strchr( tmp, '\xFE' );
         *tmp = 0;
         M_print( CONTACTCOL "%s\t\t\t", data );
         tmp++;
         data = tmp;
         tmp = strchr( tmp, '\xFE' );
         *tmp = 0;
         M_print( MESSCOL "%s" NOCOL "\n" , data );
         tmp++;
      }
	}
   else
   {
      rus_conv ("wk",data);
      if ( UIN2nick( uin ) != NULL )
         log_event( LOG_MESS, "You received instant message from %s\n%s\n", UIN2nick(uin), data );
      else
         log_event( LOG_MESS, "You received instant message from %d\n%s\n", uin, data );
      M_print( MESSCOL "\n%s", data );
      M_print( NOCOL " " );
   }
}
