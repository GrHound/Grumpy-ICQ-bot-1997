/*********************************************
**********************************************
This file has the "ui" functions that read input
and send messages etc.

This software is provided AS IS to be used in
whatever way you see fit and is placed in the
public domain.

Author : Matthew Smith April 23, 1998
Contributors : Nicolas Sahlqvist April 27, 1998
               Michael Ivey May 4, 1998
               Ulf Hedlund -- Windows Support
               Michael Holzt May 5, 1998
Changes :
22-6-98 Added the save and alter command and the
        new implementation of auto
**********************************************
**********************************************/

#include "micq.h"
#include "datatype.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
  #include <winsock2.h>
#else
  #include <unistd.h>
  #include <netinet/in.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <sys/time.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include "mreadline.h"
#endif
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "ui.h"

USER_INFO_STRUCT user;

#ifdef UNIX
//char *strdup( const char * );
//int strcasecmp( const char *, const char * );
#endif

BOOL Do_Multiline( SOK_T sok, char *buf )
{
   static int offset=0;
   static char msg[1024];
   
   msg[ offset ] = 0;
   if ( strcmp( buf, END_MSG_STR ) == 0 )
   {
      icq_sendmsg( sok, multi_uin, msg, NORM_MESS );
      M_print( "LAMBERT Do_Multiline() icq_sendmsg()" );
      Print_UIN_Name( multi_uin );
      M_print( MESSAGE_SENT_2_STR );
      last_uin = multi_uin;
      offset = 0;
      return FALSE;
   }
   else if ( strcmp( buf, CANCEL_MSG_STR ) == 0 )
   {
      M_print( MESSAGE_CANCELED_STR );
      last_uin = multi_uin;
      offset = 0;
      return FALSE;
   }
   else
   {
      if ( offset + strlen( buf ) < 450 )
      {
         strcat( msg, buf );
         strcat( msg, "\r\n" );
         offset += strlen( buf ) + 2;
         return TRUE;
      }
      else
      {
         M_print( MESSAGE_BUFFER_FULL_STR );
         Print_UIN_Name( multi_uin );
         M_print( MESSAGE_SENT_2_STR );
         icq_sendmsg( sok, multi_uin, msg, NORM_MESS );
         last_uin = multi_uin;
         offset = 0;
         return FALSE;
      }
   }
}

void Info_Update( SOK_T sok, char *buf )
{
 
   switch ( status ) {
   case NEW_NICK:
      user.nick = strdup( (char *) buf );
   	M_print ( FIRST_NAME_UPDATE_STR );
      status++;
      break;
   case NEW_FIRST:
      user.first = strdup( (char *) buf );
   	M_print ( LAST_NAME_UPDATE_STR );
      status++;
      break;
   case NEW_LAST:
      user.last = strdup( (char *) buf );
   	M_print ( EMAIL_UPDATE_STR );
      status++;
      break;
   case NEW_EMAIL:
      user.email = strdup( (char *) buf );
   	M_print( AUTH_QUESTION_STR );
      status++;
      break;
   case NEW_AUTH:
      if ( ! strcasecmp( buf, NO_STR ) )
      {
         user.auth = FALSE;
         Update_User_Info( sok, &user );
/*         free( user.nick );
         free( user.last );
         free( user.first );
         free( user.email ); */
         status = 0;
      }
      else if ( ! strcasecmp( buf, YES_STR ) )
      {
         user.auth = TRUE;
         Update_User_Info( sok, &user );
         free( user.nick );
         free( user.last );
         free( user.first );
         free( user.email );
         status = 0;
      }
      else
      {
         M_print( YESNO_RESPONSE_STR );
      	M_print( AUTH_QUESTION_STR );
      }
      break;
   }
}

void User_Search( SOK_T sok, char *buf )
{
	switch (status) {
	case SRCH_START:
		M_print ( "Enter the Users E-mail address : ");
		status++;
	break;
	case SRCH_EMAIL:
		user.email = strdup( (char *) buf );
		M_print( "Enter the Users Nick : ");
		status++;
	break;
	case SRCH_NICK:
		user.nick = strdup( (char *) buf );
		M_print ( "Enter The Users First Name : ");
		status++;
	break;
	case SRCH_FIRST:
		user.first = strdup( (char *) buf );
		M_print ( "Enter The Users Last Name : ");
		status++;
	break;
	case SRCH_LAST:
		user.last = strdup( (char *) buf );
		start_search( sok, user.email, user.nick, 
				user.first, user.last );
		status = 0;
	break;
	}
}

BOOL Do_Multiline_All( SOK_T sok, char *buf )
{
   static int offset=0;
   static char msg[1024];
   char * temp;
   int i;
   
   msg[ offset ] = 0;
   if ( strcmp( buf, END_MSG_STR ) == 0 )
   {
      for ( i=0; i < Num_Contacts; i++ ) {
         temp = strdup ( msg );
         icq_sendmsg( sok, Contacts[i].uin, temp, MRNORM_MESS );
	 free( temp );
      }
      M_print( "Message sent!" );
      offset = 0;
      return FALSE;
   }
   else if ( strcmp( buf, CANCEL_MSG_STR ) == 0 )
   {
      M_print( MESSAGE_CANCELED_STR );
      offset = 0;
      return FALSE;
   }
   else
   {
      if ( offset + strlen( buf ) < 450 )
      {
         strcat( msg, buf );
         strcat( msg, "\r\n" );
         offset += strlen( buf ) + 2;
         return TRUE;
      }
      else
      {
         M_print( MESSAGE_BUFFER_FULL_STR );
         for ( i=0; i < Num_Contacts; i++ ) {
	    temp = strdup( msg );
            icq_sendmsg( sok, Contacts[i].uin, temp, MRNORM_MESS );
	    free( temp );
	 }
         offset = 0;
         return FALSE;
      }
   }
}

/******************************************************
Read a line of input and processes it.
*******************************************************/
void Get_Input( SOK_T sok )
{
   char buf[1024]; /* This is hopefully enough */
   char *cmd;
   char *arg1;
   char *arg2;
   int i;
   
   memset( buf, 0, 1024 );
   R_getline( buf, 1024 );
   buf[1023]=0; /* be safe */
   if ( status == 1 )
   {
      if ( ! Do_Multiline( sok, buf ) )
      {
         status = 2;
      } else {
	   R_doprompt ( MSG_PROMPT_STR );
   	}
   }
   else if ( status == 3 )
   {
      if ( ! Do_Multiline_All( sok, buf ) ) {
         status = 2;
      } else {
   	   R_doprompt ( MSGA_PROMPT_STR );
   	}
   }
   else if ( ( status >= NEW_NICK) && ( status <= NEW_AUTH) )
   {  Info_Update( sok, buf ); }
   else if ( ( status >= SRCH_START) && ( status <= SRCH_LAST) )
   { User_Search( sok, buf ); }
   else
   {
      if ( buf[0] != 0 )
      {
        if ( '!' == buf[0] )
        {
	   R_pause ();
	   //system( &buf[1] );
	   R_resume ();
           Prompt();
           return;
        }
         cmd = strtok( buf, " \n\t" );
        if ( NULL == cmd )
        {
           Prompt();
           return;
        }
         /* goto's removed and code fixed by Paul Laufer. Enjoy! */
         if ( ( strcasecmp( cmd , "quit" ) == 0 ) ||
              ( strcasecmp( cmd , "/quit" ) == 0 ) )
            {  Quit = TRUE; }
         else if ( strcasecmp( cmd, quit_cmd ) == 0 )
            {  Quit = TRUE; }
         else if ( strcasecmp( cmd, sound_cmd ) == 0 )
         {
            if ( SOUND_ON == Sound )
            {
               Sound = SOUND_OFF;
               M_print( "Sound" SERVCOL " OFF" NOCOL ".\n" );
            }
            else if ( SOUND_OFF == Sound )
            {
               Sound = SOUND_ON;
               M_print( "Sound" SERVCOL  " ON" NOCOL ".\n" );
            }
         }
         else if ( strcasecmp( cmd, "bot" ) == 0 )
            {  
              if(BotMode) {
              	BotMode = 0;
              	M_print( "BotMode OFF" );
	        auto_resp = FALSE;
                M_print( "Automatic replies are off.\n" );
              } else {
              	BotMode = 1;
              	M_print( "BotMode ON" );
	        auto_resp = TRUE;
                M_print( "Automatic replies are on.\n" );
              }
           
            }
         else if ( strcasecmp( cmd, change_cmd ) == 0 )
            {  Change_Function( sok ); }
         else if ( strcasecmp( cmd, rand_cmd ) == 0 )
            {  Random_Function( sok ); }
         else if ( strcasecmp( cmd, "set" ) == 0 )
            {  Random_Set_Function( sok ); }
            else if ( ! strcasecmp( cmd, color_cmd ) )
            { 
               Color = !Color;
               if ( Color )
               {
#ifdef FUNNY_MSGS
                  M_print( SERVCOL "Being " MESSCOL "all " CLIENTCOL
		  "colorful " NOCOL
		  "and " SERVCOL "cute " CONTACTCOL "now\n" NOCOL );
#else
                   M_print( "Color is " MESSCOL "on" NOCOL ".\n" );
#endif
               }
               else
               {
#ifdef FUNNY_MSGS
                  M_print( SERVCOL "Dull colorless mode on, resistance is futile\n" NOCOL );
#else
                   M_print( "Color is " MESSCOL "off" NOCOL ".\n" );
#endif
               }
            }
	      else if ( ! strcasecmp( cmd, online_cmd ) ) /* online command */
   	      {   CHANGE_STATUS( STATUS_ONLINE ); }
	      else if ( ! strcasecmp( cmd, away_cmd ) ) /* away command */
	         {   CHANGE_STATUS( STATUS_AWAY ); }
	      else if ( ! strcasecmp( cmd, na_cmd ) ) /* Not Available command */
   	      {   CHANGE_STATUS( STATUS_NA ); }
	      else if ( ! strcasecmp( cmd, occ_cmd ) ) /* Occupied command */
	         {   CHANGE_STATUS( STATUS_OCCUPIED ); }
	      else if ( ! strcasecmp( cmd, dnd_cmd ) ) /* Do not Disturb  command */
   	      {   CHANGE_STATUS( STATUS_DND ); }
	      else if ( ! strcasecmp( cmd, ffc_cmd ) ) /* Free For Chat command */
   	      {   CHANGE_STATUS( STATUS_FREE_CHAT ); }
	      else if ( ! strcasecmp( cmd, inv_cmd ) ) /* Invisible command */
   	      {   CHANGE_STATUS( STATUS_INVISIBLE ); }
	      else if ( ! strcasecmp( cmd, search_cmd ) )
   	   {
             arg1 = strtok( NULL, "\n" );
             if ( arg1 == NULL )
             {
		      status = SRCH_START;
		      User_Search( sok, buf );
             }
             else
             {
   	          start_search( sok, arg1, "", "", "" );
             }
	      }
         else if ( ! strcasecmp( cmd, status_cmd ) ) 
         {  Show_Status(); }
         else if ( ! strcasecmp( cmd, list_cmd ) ) 
         {  Show_Quick_Status(); }
         else if ( ! strcasecmp( cmd, msga_cmd ) )
         {
            status = 3;
            R_doprompt( MSGA_PROMPT_STR );
         }
         else if ( ! strcasecmp( cmd, reply_cmd ) ) /* reply command */
         {
            Reply_Function( sok );
         }
         else if ( ! strcasecmp( cmd, "reg" ) )
         {
            arg1 = strtok( NULL, "" );
            if ( arg1 != NULL )
            {  reg_new_user( sok, arg1 ); }
         }
         else if ( ! strcasecmp( cmd, "pass" ) )
         {
            arg1 = strtok( NULL, "" );
            if ( arg1 != NULL )
            {  Change_Password( sok, arg1 ); }
         }
         else if ( ! strcasecmp( cmd, again_cmd ) ) /* again command */
         {  Again_Function( sok ); }
         else if ( ! strcasecmp( cmd, "clear"))
   	   { clrscr(); }
         else if ( ! strcasecmp( cmd, info_cmd ) )
         {  Info_Function( sok ); }
         else if ( ! strcasecmp( cmd, "ver" ) )
         { M_print( "Micq Version : " MESSCOL MICQ_VERSION NOCOL " Compiled on "__TIME__ " "  __DATE__"\n" ); }
         else if ( ! strcasecmp( cmd, add_cmd ) )
         {
            arg1 = strtok( NULL, " \t" );
            if ( arg1 != NULL )
            {
               uin = atoi( arg1 );
               arg1 = strtok( NULL, "" );
               if ( arg1 != NULL )
               {
                  Add_User( sok, uin, arg1 );
                  M_print( "%s added.\n", arg1 );
               }
            }
            else
            {
               M_print( SERVCOL "Must specify a nick name" NOCOL "\n" );
            }
         }
         else if ( strcasecmp( cmd, "verbose" ) == 0 )
         { Verbose_Function(); }
         else if ( strcasecmp( cmd, "rinfo" ) == 0 )
         {
            Print_UIN_Name( last_recv_uin );
            M_print( "'s IP address is " );
            Print_IP( last_recv_uin );
            M_print( "\tThe port is %d\n",(WORD) Get_Port( last_recv_uin ) );
/*            M_print( "\n" );*/
            send_info_req( sok, last_recv_uin );
            send_ext_info_req( sok, last_recv_uin );
         }
         else if ( ( strcasecmp( cmd, "/help" ) == 0 ) ||/* Help command */
                   ( strcasecmp( cmd, "help" ) == 0 ) )
         {  Help_Function(); }
         else if ( strcasecmp( cmd, auth_cmd ) == 0 )
         {
             arg1 = strtok( NULL, "" );
             if ( arg1 == NULL )
             {
                M_print( "Need uin to send to" );
             }
             else
             {
              uin = nick2uin( arg1 );
              if ( -1 == uin )
              {
                 M_print( "%s not recognized as a nick name", arg1 );
              }
              else icq_sendauthmsg( sok, uin );
             }         
         }
         else if ( strcasecmp( cmd, message_cmd ) == 0 ) /* "/msg" */
         {
            Message_Function( sok );
         }
	      else if ( strcasecmp( cmd, url_cmd ) == 0 ) /* "/msg" */
	      {
		      arg1 = strtok( NULL, " " );
		      if ( arg1 == NULL )
		      {
			      M_print( "Need uin to send to\n" );
		      }
		      else
		      {
			      uin = nick2uin( arg1 );
			      if ( uin == -1 )
			      {
				      M_print( "%s not recognized as a nick name\n", arg1 );
			      }
			      else
			      {
				      arg1 = strtok( NULL, " " );
				      last_uin = uin;
				      if ( arg1 != NULL )
				      {
					      arg2 = strtok(NULL, "");
/*					      sM_print(arg1, "%s%c%s", arg1, '\xFE', arg2);*/
					      icq_sendurl( sok, uin, arg1, arg2 );
					      M_print( "URL sent to ");
					      Print_UIN_Name( last_uin );
					      M_print( "!");
				      } else {
					      M_print("Need URL please.\n");
				      } 
			      }
		      }
	      }
         else if ( ! strcasecmp( cmd, alter_cmd ) ) /* alter command */
         {  Alter_Function(); }
         else if ( ! strcasecmp( cmd, save_cmd ) ) /* save command */
         {
            i=Save_RC();
            if (i==-1)
               M_print("Sorry saving your personal reply messages went wrong!\n");
            else
               M_print("Your personal settings have been saved!\n");
         }
         else if ( ! strcasecmp( cmd, "update" ) )
         {
            status = NEW_NICK;
            M_print( NICK_NAME_UPDATE_STR );
         }
         else if ( strcasecmp( cmd, auto_cmd ) == 0 )
         {  Auto_Function( sok ); }
         else 
         {  M_print( "Unknown command %s, type /help for help.\n", cmd ); }
      }
   }
   multi_uin = last_uin;
   if ( ( status == 0 ) || ( status == 2) )
   {
      if ( ! Quit )
#ifndef USE_MREADLINE
         Prompt();
//         Soft_Prompt();
#else
         Prompt();
#endif
   }
}

/**************************************************************
most detailed contact list display
***************************************************************/
static void Show_Status( void )
{
   int i;

   M_print( W_SEPERATOR );
   M_print( W_STATUS_STR );
   Print_Status( Current_Status );
   M_print( "\n" );
   /*  First loop sorts thru all offline users */
   M_print( W_SEPERATOR );
   M_print( W_OFFLINE_STR "\n" );
   for ( i=0; i< Num_Contacts; i++ )
   {
      if ( ( S_DWORD )Contacts[ i ].uin > 0 )
      {
         if ( FALSE == Contacts[ i ].invis_list )
	 {
            if ( Contacts[ i ].status == STATUS_OFFLINE )
            {
               if ( Contacts[i].vis_list ) {
        	  M_print( "%s*%s", SERVCOL, NOCOL );
               } else {
        	  M_print( " " );
               }
               M_print( "%8ld=", Contacts[ i ].uin );
               M_print( CONTACTCOL "%-20s\t%s(", Contacts[ i ].nick, MESSCOL );
               Print_Status( Contacts[ i ].status );
               M_print( ")%s", NOCOL );
               if ( -1L != Contacts[ i ].last_time )
               {
                  M_print( W_LAST_ONLINE_STR , ctime( (time_t *) &Contacts[ i ].last_time ) );
               }
               else
               {
        	  M_print( W_LAST_ON_UNKNOWN_STR "\n" );
        	  /* if time is unknow they can't be logged on cause we */
        	  /* set the time at login */
               }
            }
	 }
      }
   }
   /* The second loop displays all the online users */
   M_print( W_SEPERATOR );
   M_print( W_ONLINE_STR "\n" );
   for ( i=0; i< Num_Contacts; i++ )
   {
      if ( ( S_DWORD )Contacts[ i ].uin > 0 )
      {
         if ( FALSE == Contacts[ i ].invis_list )
	 {
            if ( Contacts[ i ].status != STATUS_OFFLINE )
            {
               if ( Contacts[i].vis_list ) {
        	  M_print( "%s*%s", SERVCOL, NOCOL );
               } else {
        	  M_print( " " );
               }
               M_print( "%8ld=", Contacts[ i ].uin );
               M_print( CONTACTCOL "%-20s\t%s(", Contacts[ i ].nick, MESSCOL );
               Print_Status( Contacts[ i ].status );
               M_print( ")%s", NOCOL );
               if ( -1L != Contacts[ i ].last_time )
               {
        	  if ( Contacts[ i ].status  == STATUS_OFFLINE )
                     M_print( W_LAST_ONLINE_STR, ctime( (time_t *) &Contacts[ i ].last_time ) );
        	  else
                     M_print( W_ONLINE_SINCE_STR, ctime( (time_t *) &Contacts[ i ].last_time )  );
               }
               else
               {
        	  M_print( W_LAST_ON_UNKNOWN_STR "\n" );
        	  /* if time is unknow they can't be logged on cause we */
        	  /* set the time at login */
               }
            }
	 }
      }
   }
   M_print( W_SEPERATOR );
}

/***************************************************************
nice clean "w" display
****************************************************************/
void Show_Quick_Status( void )
{
   int i;
   
   M_print( "" W_SEPERATOR );
   M_print( "%lu: ", UIN );
   M_print( W_STATUS_STR );
   Print_Status( Current_Status );
   M_print( "\n" );
   M_print( W_SEPERATOR );
   M_print( W_OFFLINE_STR "\n" );
   /*  First loop sorts thru all offline users */
   /*  This comes first so that if there are many contacts */
   /*  The online ones will be less likely to scroll off the screen */
   for ( i=0; i< Num_Contacts; i++ )
   {
      if ( ( S_DWORD )Contacts[ i ].uin > 0 )
      {
         if ( FALSE == Contacts[ i ].invis_list )
	 {
            if ( Contacts[ i ].status == STATUS_OFFLINE )
            {
               if ( Contacts[i].vis_list ) {
        	  M_print( "%s*%s", SERVCOL, NOCOL );
               } else {
        	  M_print( " " );
               }
               M_print( CONTACTCOL "%-20s\t" MESSCOL "(" , Contacts[ i ].nick );
               Print_Status( Contacts[ i ].status );
               M_print( ")" NOCOL "\n" );
            }
	 }
      }
   }
   /* The second loop displays all the online users */
   M_print( W_SEPERATOR );
   M_print( W_ONLINE_STR "\n" );
   for ( i=0; i< Num_Contacts; i++ )
   {
      if ( ( S_DWORD )Contacts[ i ].uin > 0 )
      {
         if ( FALSE == Contacts[ i ].invis_list )
	 {
            if ( Contacts[ i ].status != STATUS_OFFLINE )
            {
               if ( Contacts[i].vis_list ) {
        	  M_print( "%s*%s", SERVCOL, NOCOL );
               } else {
        	  M_print( " " );
               }
               M_print( CONTACTCOL "%-20s\t" MESSCOL "(", Contacts[ i ].nick );
               Print_Status( Contacts[ i ].status );
               M_print( ")" NOCOL "\n" );
            }
	 }
      }
   }
   M_print( W_SEPERATOR );
}

/***************************************************
Do not call unless you've already made a call to strtok()
****************************************************/
static void Change_Function( SOK_T sok )
{
   char *arg1;
   arg1 = strtok( NULL, " \n\r" );
   if ( arg1 == NULL )
   {
      M_print(  CLIENTCOL "Status modes: \n" );
      M_print( "Status %s\t%d\n", STATUS_ONLINE_STR, STATUS_ONLINE );
      M_print( "Status %s\t%d\n", STATUS_AWAY_STR, STATUS_AWAY );
      M_print( "Status %s\t%d\n", STATUS_DND_STR, STATUS_DND );
      M_print( "Status %s\t%d\n", STATUS_NA_STR, STATUS_NA );
      M_print( "Status %s\t%d\n", STATUS_FFC_STR, STATUS_FREE_CHAT );
      M_print( "Status %s\t%d\n", STATUS_OCCUPIED_STR, STATUS_OCCUPIED );
      M_print( "Status %s\t%d", STATUS_INVISIBLE_STR, STATUS_INVISIBLE );
      M_print( NOCOL "\n" );
   }
   else
   {
      icq_change_status( sok, atoi( arg1 ) );
      Print_Status( Current_Status );
/*      M_print( "\n" );*/
   }
}

/***************************************************
Do not call unless you've already made a call to strtok()
****************************************************/
static void Random_Function( SOK_T sok )
{
   char *arg1;
   arg1 = strtok( NULL, " \n\r" );
   if ( arg1 == NULL )
   {
      M_print(  CLIENTCOL "Groups: \n" );
      M_print( "General                    1\n" );
      M_print( "Romance                    2\n" );
      M_print( "Games                      3\n" );
      M_print( "Students                   4\n" );
      M_print( "20 something               6\n" );
      M_print( "30 something               7\n" );
      M_print( "40 something               8\n" );
      M_print( "50+                        9\n" );
      M_print( "Man chat requesting women 10\n" );
      M_print( "Woman chat requesting men 11\n" );
      M_print( "Micq                      49 (might not work but try it)" );
      M_print( NOCOL "\n" );
   }
   else
   {
      icq_rand_user_req( sok, atoi( arg1 ) );
/*      M_print( "\n" );*/
   }
}

/***************************************************
Do not call unless you've already made a call to strtok()
****************************************************/
static void Random_Set_Function( SOK_T sok )
{
   char *arg1;
   arg1 = strtok( NULL, " \n\r" );
   if ( arg1 == NULL )
   {
      M_print(  CLIENTCOL "Groups: \n" );
      M_print( "None                      -1\n" );
      M_print( "General                    1\n" );
      M_print( "Romance                    2\n" );
      M_print( "Games                      3\n" );
      M_print( "Students                   4\n" );
      M_print( "20 something               6\n" );
      M_print( "30 something               7\n" );
      M_print( "40 something               8\n" );
      M_print( "50+                        9\n" );
      M_print( "Man chat requesting women 10\n" );
      M_print( "Woman chat requesting men 11\n" );
      M_print( "Micq                      49 (might not work but try it)" );
      M_print( NOCOL "\n" );
   }
   else
   {
      icq_rand_set( sok, atoi( arg1 ) );
/*      M_print( "\n" );*/
   }
}

/*****************************************************************
Displays help information.
******************************************************************/
static void Help_Function( void )
{
   char *arg1;
   arg1 = strtok( NULL, " \n\r" );
   
   if ( arg1 == NULL ){
       M_print( CLIENTCOL SELECT_GROUP_STR "\n" );
       M_print( CLIENT_CAT_STR "\t-\t" CLIENT_HELP_STR "\n" );
       M_print( MESSAGE_CAT_STR "\t-\t" MESSAGE_HELP_STR "\n" );
       M_print( USER_CAT_STR "\t-\t" USER_HELP_STR "\n" );
       M_print( ACCOUNT_CAT_STR "\t-\t" ACCOUNT_HELP_STR "\n" );
   } else if ( !strcasecmp( arg1, CLIENT_CAT_STR ) ){
	    M_print( MESSCOL "verbose #" NOCOL VERBOSE_HELP_STR);
            M_print( MESSCOL "clear" NOCOL CLEAR_HELP_STR );
            M_print( MESSCOL "%s" NOCOL SOUND_HELP_STR, sound_cmd);
            M_print( MESSCOL "%s" NOCOL COLOR_HELP_STR, color_cmd);
            M_print( MESSCOL "%s\t" NOCOL QUIT_HELP_STR,quit_cmd );
            M_print( MESSCOL "%s" NOCOL AUTO1_HELP_STR,auto_cmd );
            M_print( MESSCOL "%s [on|off]" NOCOL AUTO2_HELP_STR,auto_cmd );
            M_print( MESSCOL "%s <status> <message>" NOCOL AUTO3_HELP_STR,auto_cmd );
            M_print( MESSCOL "%s <old cmd> <new cmd>" NOCOL ALTER_HELP_STR ,alter_cmd);
	    M_print( CLIENTCOL "\t! as the first character of a command will execute\n" );
	    M_print( CLIENTCOL "\ta shell command (e.g. \"!ls\"  \"!dir\" \"!mkdir temp\")" NOCOL "\n" );
   } else if ( !strcasecmp( arg1,MESSAGE_CAT_STR ) ){
            M_print( MESSCOL "%s <uin>" NOCOL AUTH_HELP_STR, auth_cmd );
            M_print( MESSCOL "%s <uin>/<message>" NOCOL MSG_HELP_STR, message_cmd );
            M_print( MESSCOL "%s <uin> <url> <message>" NOCOL URL_HELP_STR, url_cmd );
            M_print( MESSCOL "%s\t\t" NOCOL MSGA_HELP_STR, msga_cmd );
            M_print( MESSCOL "%s <message>" NOCOL AGAIN_HELP_STR, again_cmd);
            M_print( MESSCOL "%s <message>" NOCOL REPLY_HELP_STR, reply_cmd);
	    M_print( CLIENTCOL "\tuin can be either a number or the nickname of the user.\n" );
            M_print( CLIENTCOL "\tSending a blank message will put the client into multiline mode.\n\tUse . on a line by itself to end message.\n" );
	    M_print( "\tUse # on a line by itself to cancel the message." NOCOL "\n" );
   } else if ( !strcasecmp( arg1, USER_CAT_STR ) ){
            M_print( MESSCOL "%s <uin>" NOCOL AUTH_HELP_STR, auth_cmd );
	    M_print( MESSCOL "%s [#]" NOCOL RAND_HELP_STR, rand_cmd );
	    M_print( MESSCOL "pass <secret>" NOCOL PASS_HELP_STR );
            M_print( MESSCOL "%s" NOCOL LIST_HELP_STR,list_cmd);
            M_print( MESSCOL "%s <uin>" NOCOL INFO_HELP_STR, info_cmd );
            M_print( MESSCOL "%s [email@host]" NOCOL SEARCH_HELP_STR, search_cmd );
            M_print( MESSCOL "%s <uin> <nick>" NOCOL ADD_HELP_STR, add_cmd );
   } else if ( !strcasecmp( arg1, ACCOUNT_CAT_STR ) ){
            M_print( MESSCOL "%s [#]" NOCOL CHANGE_HELP_STR,change_cmd);
	    M_print( MESSCOL "reg password" NOCOL REG_HELP_STR );
	    M_print( MESSCOL "%s" NOCOL ONLINE_HELP_STR,online_cmd);
	    M_print( MESSCOL "%s" NOCOL AWAY_HELP_STR,away_cmd);
            M_print( MESSCOL "%s" NOCOL NA_HELP_STR,na_cmd);
            M_print( MESSCOL "%s" NOCOL OCC_HELP_STR,occ_cmd);
            M_print( MESSCOL "%s" NOCOL DND_HELP_STR,dnd_cmd);
            M_print( MESSCOL "%s" NOCOL FFC_HELP_STR,ffc_cmd);
            M_print( MESSCOL "%s" NOCOL INV_HELP_STR,inv_cmd);
	    M_print( MESSCOL "%s" NOCOL UPDATE_HELP_STR,update_cmd);
	    M_print( MESSCOL "set [#]" NOCOL SET_RAND_HELP_STR );
   }
}

/****************************************************
Retrieves info a certain user
*****************************************************/
static void Info_Function( SOK_T sok )
{
   char * arg1;

   arg1 = strtok( NULL, "" );
   if ( arg1 == NULL )
   {
      M_print( "Need uin to send to\n" );
      return;
   }
   uin = nick2uin( arg1 );
   if ( -1 == uin )
   {
      M_print( NICK_NOT_FOUND_STR "\n", arg1 );
      return;
   }
   M_print( "%s's IP address is ", arg1 );
   Print_IP( uin );
   M_print( "\tThe port is %d\n",(WORD) Get_Port( uin ) );
   M_print( "" );
   send_info_req( sok, uin );
   send_ext_info_req( sok, uin );
}

/**********************************************************
Handles automatic reply messages
***********************************************************/
static void Auto_Function( SOK_T sok )
{
   char *cmd;
   char *arg1;
   
   cmd = strtok( NULL, "" );
   if ( cmd == NULL )
   {
      M_print( "Automatic replies are %s\n", auto_resp ? "On" : "Off" );
      M_print( "The Do not disturb message is: %s\n", auto_rep_str_dnd );
      M_print( "The Away message is          : %s\n", auto_rep_str_away );
	   M_print( "The Not available message is  : %s\n", auto_rep_str_na );
 	   M_print( "The Occupied message is      : %s\n", auto_rep_str_occ );
 	   M_print( "The Invisible message is     : %s\n", auto_rep_str_inv );
 	   return;
   }
   else if ( strcasecmp( cmd, "on" ) == 0 )
   {
      auto_resp = TRUE;
      M_print( "Automatic replies are on.\n" );
   }
   else if ( strcasecmp( cmd, "off" ) == 0 )
   {
      auto_resp = FALSE;
      M_print( "Automatic replies are off.\n" );
   }
   else
   {
       M_print( "Automatic reply setting\n" );
       arg1 = strtok( cmd," ");
 	   if (arg1 == NULL)
 	   {
 		   M_print( "Sorry wrong syntax, can't find a status somewhere.\r\n");
         return;
 	   }
 		if ( ! strcasecmp (arg1 , dnd_cmd ) )
 		{
 			 cmd = strtok( NULL,"");
 			 strcpy( auto_rep_str_dnd, cmd);
 		}
 		else if ( !strcasecmp (arg1 ,away_cmd ))
 	   {
 		  cmd = strtok( NULL,"");
 		  strcpy( auto_rep_str_away,cmd);
 	   }
 	   else if ( !strcasecmp (arg1,na_cmd))
 	   {
 		    cmd =strtok(NULL,"");
 		    strcpy(auto_rep_str_na,cmd);
 	   }
 	   else if ( !strcasecmp (arg1,occ_cmd))
 	   {
 		    cmd = strtok(NULL,"");
 		    strcpy (auto_rep_str_occ,cmd);
 	   }
 	   else if (!strcasecmp (arg1,inv_cmd))
 	   {
 		   cmd = strtok(NULL,"");
 		   strcpy (auto_rep_str_inv,cmd);
 	   }
 	   else
 		   M_print ("Sorry wrong syntax. Read tha help man!\n");
 	}
}

/*************************************************************
Alters one of the commands
**************************************************************/
static void Alter_Function( void )
{
   char * cmd;
   
   cmd = strtok (NULL," ");
   if ( cmd == NULL )
   {
     M_print( "Need a command to alter!\n" );
     return;
   }
   if ( ! strcasecmp(cmd, auto_cmd))
     strncpy(auto_cmd,strtok(NULL," \n\t"),16);
   else if ( !strcasecmp(cmd ,message_cmd))
     strncpy(message_cmd,strtok(NULL," \n\t"),16);
   else if (!strcasecmp(cmd,add_cmd))
     strncpy(add_cmd,strtok(NULL," \n\t"),16);
   else if (!strcasecmp(cmd,info_cmd))
     strncpy(info_cmd,strtok(NULL," \n\t"),16);
   else if (!strcasecmp(cmd,quit_cmd))
     strncpy(quit_cmd,strtok(NULL," \n\t"),16);
   else if (!strcasecmp(cmd,reply_cmd))
        strncpy(reply_cmd,strtok(NULL," \n\t"),16);
   else if (!strcasecmp(cmd,again_cmd))
           strncpy(again_cmd,strtok(NULL," \n\t"),16);
   else if (!strcasecmp(cmd,list_cmd))
           strncpy(list_cmd,strtok(NULL," \n\t"),16);
   else if (!strcasecmp(cmd,away_cmd))
           strncpy(away_cmd,strtok(NULL," \n\t"),16);
   else if (!strcasecmp(cmd,na_cmd))
           strncpy(na_cmd,strtok(NULL," \n\t"),16);
   else if (!strcasecmp(cmd,dnd_cmd))
           strncpy(dnd_cmd,strtok(NULL," \t\n"),16);
   else if (!strcasecmp(cmd,online_cmd))
           strncpy(online_cmd,strtok(NULL," \n\t"),16);
   else if (!strcasecmp(cmd,occ_cmd))
           strncpy(occ_cmd,strtok(NULL," \t\n"),16);
   else if (!strcasecmp(cmd,ffc_cmd))
           strncpy(ffc_cmd,strtok(NULL," \t\n"),16);
   else if (!strcasecmp(cmd,inv_cmd))
            strncpy(inv_cmd,strtok(NULL," \t\n"),16);
   else if (!strcasecmp(cmd,status_cmd))
           strncpy(status_cmd,strtok(NULL," \t\n"),16);
   else if (!strcasecmp(cmd,auth_cmd))
           strncpy(status_cmd,strtok(NULL," \n\t"),16);
   else if (!strcasecmp(cmd,change_cmd))
           strncpy(change_cmd,strtok(NULL," \n\t"),16);
   else if (!strcasecmp(cmd,search_cmd))
           strncpy(search_cmd,strtok(NULL," \t\n"),16);
   else if (!strcasecmp(cmd,save_cmd))
           strncpy(save_cmd,strtok(NULL," \t\n"),16);
   else if (!strcasecmp(cmd,alter_cmd))
           strncpy(alter_cmd,strtok(NULL," \t\n"),16);
   else if (!strcasecmp(cmd,msga_cmd))
          strncpy(msga_cmd,strtok(NULL," \n\t"),16);
   else
     M_print("Type help to see your current command, because this  one you typed wasn't one!\n");
}

/*************************************************************
Processes user input to send a message to a specified user
**************************************************************/
static void Message_Function( SOK_T sok )
{
   char * arg1;
   
   arg1 = strtok( NULL, UIN_DELIMS );
   if ( arg1 == NULL )
   {
      M_print( "Need uin to send to\n" );
      return;
   }
   uin = nick2uin( arg1 );
   if ( -1 == uin )
   {
      M_print( NICK_NOT_FOUND_STR "\n", arg1 );
      return;
   }
   arg1 = strtok( NULL, "" );
   last_uin = uin;
   if ( arg1 != NULL )
   {
      icq_sendmsg( sok, uin, arg1, NORM_MESS );
      M_print( "LAMBERT MessageFunction() icq_sendmsg()" );
      Print_UIN_Name( last_uin );
      M_print( MESSAGE_SENT_2_STR );
   }
   else
   {
      status = 1;
      R_doprompt( MSG_PROMPT_STR );
   }
}

/*******************************************************
Sends a reply message to the last person to message you.
********************************************************/
static void Reply_Function( SOK_T sok )
{
   char * arg1;
   
   if ( last_recv_uin == 0 )
   {
      M_print( "Must receive a message first\n" );
      return;
   }
   arg1 = strtok( NULL, "" );
   last_uin = last_recv_uin;
   if ( arg1 != NULL )
   {
      icq_sendmsg( sok, last_recv_uin, arg1, NORM_MESS );
      M_print( "LAMBERT ReplyFunction() icq_sendmsg()\n" );
      Print_UIN_Name( last_recv_uin );
      M_print( MESSAGE_SENT_2_STR );
   }
   else
   {
      status = 1;
      R_doprompt( MSG_PROMPT_STR );
   }
}

static void Again_Function( SOK_T sok )
{
   char * arg1;
   
   if ( last_uin == 0 )
   {
      M_print( "Must write one message first\n" );
      return;
   }
   arg1 = strtok( NULL, "" );
   if ( arg1 != NULL )
   {
      icq_sendmsg( sok, last_uin, arg1, NORM_MESS );
      M_print( "LAMBERT: AgainFunction() icq_send_msg()" );
      Print_UIN_Name( last_uin );
      M_print( MESSAGE_SENT_2_STR );					 
   } else {
      status = 1;
      R_doprompt( MSG_PROMPT_STR );
   }
}

static void Verbose_Function( void )
{
   char * arg1;
   
   arg1 = strtok( NULL, "" );
   if ( arg1 != NULL )
   {
      Verbose = atoi( arg1 );
   }
   M_print( VERBOSE_LEVEL_STR, Verbose );
}
