/*********************************************
**********************************************
Header file for ICQ protocol structres and
constants

This software is provided AS IS to be used in
whatever way you see fit and is placed in the
public domain.

Author : Matthew Smith April 19, 1998
Contributors : None yet

Changes :
   4-21-98 Increase the size of data associtated
            with the packets to enable longer messages. mds
   4-22-98 Added function prototypes and extern variables mds
   4-22-98 Added SRV_GO_AWAY code for bad passwords etc.
**********************************************
**********************************************/
#define MICQ_VERSION "0.4.0 Now wit V5 and readline.\t"

#include "datatype.h"
#include <stdlib.h>

/*********  Leeched from Xicq :) xtrophy@it.dk ********/
/*********  changed to use escape codes like you should :) ***/
/*********  changed colors ***********************************/
#ifdef ANSI_COLOR
/* Last 2 digit number selects the color */
/* Experiment and let me know if you come up with */
/* A better scheme */
   #define SERVCOL         "\x1B[0;31m"
   #define MESSCOL         "\x1B[1;34m"
   #define CONTACTCOL      "\x1B[1;35m"
   #define CLIENTCOL       "\x1B[0;32m"
   #define NOCOL           "\x1B[0m"
#else
   #define SERVCOL         ""
   #define MESSCOL         ""
   #define CONTACTCOL      ""
   #define CLIENTCOL       ""
   #define NOCOL           ""
#endif
#include "lang.h"

#define SOUND_ON 1
#define SOUND_OFF 0
#define SOUND_CMD 2

#define ICQ_VER 0x0005

#define STDIN 0
#define STDOUT 1
#define STDERR 2

/* The maximum number of contacts to put in a single packet. */
#define MAX_CONTS_PACKET 90

#if ICQ_VER == 0x0005
   typedef struct
   {
      BYTE dummy[2]; /* to fix alignment problem */
      BYTE ver[2];
      BYTE zero[4];
      BYTE  UIN[4];
      BYTE session[4];
      BYTE cmd[2];
      BYTE seq2[2];
      BYTE seq[2];
      BYTE checkcode[4];
   } ICQ_pak, *ICQ_PAK_PTR;

   typedef struct
   {
      BYTE dummy[2]; /* to fix alignment problem */
      BYTE ver[2];
      BYTE zero;
      BYTE session[4];
      BYTE cmd[2];
      BYTE seq[2];
      BYTE seq2[2];
      BYTE UIN[4];
      BYTE check[4];
   } SRV_ICQ_pak, *SRV_ICQ_PAK_PTR;

#define CMD_OFFSET 14
#define SEQ_OFFSET 16
#define SEQ2_OFFSET 18

#elif ICQ_VER == 0x0004

   typedef struct
   {
      BYTE dummy[2]; /* to fix alignment problem */
      BYTE ver[2];
      BYTE rand[2];
      BYTE zero[2];
      BYTE cmd[2];
      BYTE seq[2];
      BYTE seq2[2];
      BYTE  UIN[4];
      BYTE checkcode[4];
   } ICQ_pak, *ICQ_PAK_PTR;

   typedef struct
   {
      WORD dummy; /* to fix alignment problem */
      BYTE ver[2];
      BYTE cmd[2];
      BYTE seq[2];
      BYTE seq2[2];
      BYTE UIN[4];
      BYTE check[4];
   } SRV_ICQ_pak, *SRV_ICQ_PAK_PTR;

#else /* ICQ_VER */

   typedef struct
   {
      WORD dummy; /* to fix alignment problem */
      BYTE ver[2];
      BYTE cmd[2];
      BYTE seq[2];
      BYTE  UIN[4];
   } ICQ_pak, *ICQ_PAK_PTR;

   typedef struct
   {
      WORD dummy; /* to fix alignment problem */
      BYTE ver[2];
      BYTE cmd[2];
      BYTE seq[2];
   } SRV_ICQ_pak, *SRV_ICQ_PAK_PTR;

#endif  /* ICQ_VER */

typedef struct
{
   ICQ_pak  head;
   unsigned char  data[1024];
} net_icq_pak, *NET_ICQ_PTR;

typedef struct
{
   SRV_ICQ_pak  head;
   unsigned char  data[1024];
} srv_net_icq_pak, *SRV_NET_ICQ_PTR;

#define CMD_ACK               0x000A 
#define CMD_SENDM             0x010E
#define CMD_LOGIN             0x03E8
#define CMD_CONT_LIST         0x0406
#define CMD_SEARCH_UIN        0x041a
#define CMD_SEARCH_USER       0x0424
#define CMD_KEEP_ALIVE        0x042e
#define CMD_KEEP_ALIVE2       0x051e
#define CMD_SEND_TEXT_CODE    0x0438
#define CMD_LOGIN_1           0x044c
#define CMD_INFO_REQ          0x0460
#define CMD_EXT_INFO_REQ      0x046a
#define CMD_CHANGE_PW         0x049c
#define CMD_STATUS_CHANGE     0x04d8
#define CMD_LOGIN_2           0x0528
#define CMD_UPDATE_INFO       0x050A
#define CMD_UPDATE_EXT_INFO   0X04B0
#define CMD_ADD_TO_LIST       0X053C
#define CMD_REQ_ADD_LIST      0X0456
#define CMD_QUERY_SERVERS     0X04BA
#define CMD_QUERY_ADDONS      0X04C4
#define CMD_NEW_USER_1        0X04EC
#define CMD_NEW_USER_INFO     0x04A6
#define CMD_ACK_MESSAGES      0x0442
#define CMD_MSG_TO_NEW_USER   0x0456
#define CMD_REG_NEW_USER      0x03FC
#define CMD_VIS_LIST          0x06AE
#define CMD_INVIS_LIST        0x06A4
#define CMD_META_USER         0x064A
#define CMD_RAND_SEARCH       0x056E
#define CMD_RAND_SET          0x0564
#define CMD_AUTH_UPDATE       0x0514

#define SRV_ACK            0x000A
#define SRV_LOGIN_REPLY    0x005A
#define SRV_USER_ONLINE    0x006E
#define SRV_USER_OFFLINE   0x0078
#define SRV_USER_FOUND     0x008C
#define SRV_RECV_MESSAGE   0x00DC
#define SRV_END_OF_SEARCH  0x00A0
#define SRV_INFO_REPLY     0x0118
#define SRV_EXT_INFO_REPLY 0x0122
#define SRV_STATUS_UPDATE  0x01A4
#define SRV_X1             0x021C
#define SRV_X2             0x00E6
#define SRV_UPDATE_EXT     0x00C8
#define SRV_NEW_UIN        0x0046
#define SRV_NEW_USER       0x00B4
#define SRV_QUERY          0x0082
#define SRV_SYSTEM_MESSAGE 0x01C2
#define SRV_SYS_DELIVERED_MESS 0x0104
#define SRV_GO_AWAY        0x0028
#define SRV_NOT_CONNECTED  0x00F0
#define SRV_BAD_PASS       0x0064
#define SRV_TRY_AGAIN      0x00FA
#define SRV_UPDATE_FAIL    0x01EA
#define SRV_UPDATE_SUCCESS 0x01E0
#define SRV_MULTI_PACKET   0x0212
#define SRV_META_USER      0x03DE
#define SRV_RAND_USER      0x024E
#define SRV_AUTH_UPDATE    0x01F4

#define META_INFO_SET    0x3E8
#define META_INFO_REQ    0x04B0
#define META_INFO_SECURE 0x0424
#define META_INFO_PASS   0x042E
#define META_SRV_GEN    0x00C8
#define META_SRV_MORE   0x00DC
#define META_SRV_WORK   0x00D2
#define META_SRV_PASS   0x00AA


#define STATUS_OFFLINE  (-1L)
#define STATUS_ONLINE  0x00
#define STATUS_INVISIBLE 0x100
#define STATUS_NA_99        0x04
#define STATUS_NA      0x05
#define STATUS_FREE_CHAT 0x20
#define STATUS_OCCUPIED_MAC 0x10
#define STATUS_OCCUPIED 0x11
#define STATUS_AWAY    0x01
#define STATUS_DND    0x13
#define STATUS_DND_99    0x02

#define AUTH_MESSAGE  0x0008

#define USER_ADDED_MESS 0x000C
#define AUTH_REQ_MESS 0x0006
#define URL_MESS	0x0004
#define MASS_MESS_MASK  0x8000
#define MRURL_MESS	0x8004
#define NORM_MESS		0x0001
#define MRNORM_MESS	0x8001
#define CONTACT_MESS	0x0013
#define MRCONTACT_MESS	0x8013

/*#define USA_COUNTRY_CODE 0x01
#define UK_COUNTRY_CODE 0x2C*/
#if ICQ_VER == 0x0005

   typedef struct
   {
      BYTE time[4];
      BYTE port[4];
      BYTE len[2];
   } login_1, *LOGIN_1_PTR;

   typedef struct
   {
      BYTE X1[4];
      BYTE ip[4];
      BYTE  X2[1];
      BYTE  status[4];
      BYTE X3[4];
   /*   BYTE seq[2];*/
      BYTE  X4[4];
      BYTE X5[4];
      BYTE X6[4];
      BYTE X7[4];
      BYTE X8[4];
   } login_2, *LOGIN_2_PTR;

   /* those behind the // are for the spec on
    http://www.student.nada.kth.se/~d95-mih/icq/
    they didn't work for me so I changed them
    to values that did work.
    *********************************/
   /*#define LOGIN_X1_DEF 0x00000078 */
   #define LOGIN_X1_DEF 0x000000C8
/*   #define LOGIN_X2_DEF 0x04*/
   #define LOGIN_X2_DEF 0x06
   /*#define LOGIN_X3_DEF 0x00000002*/
   #define LOGIN_X3_DEF 0x00000006
   /*#define LOGIN_X4_DEF 0x00000000*/
   #define LOGIN_X4_DEF 0x00000000
   /*#define LOGIN_X5_DEF 0x00780008*/
   #define LOGIN_X5_DEF 0x00C80010
   #define LOGIN_X6_DEF 0x00000050
   #define LOGIN_X7_DEF 0x00000003
   #define LOGIN_X8_DEF 0x00000000

#elif ICQ_VER == 0x0004

   typedef struct
   {
      BYTE time[4];
      BYTE port[4];
      BYTE len[2];
   } login_1, *LOGIN_1_PTR;

   typedef struct
   {
      BYTE X1[4];
      BYTE ip[4];
      BYTE  X2[1];
      BYTE  status[4];
      BYTE X3[4];
   /*   BYTE seq[2];*/
      BYTE  X4[4];
      BYTE X5[4];
   } login_2, *LOGIN_2_PTR;

   /* those behind the // are for the spec on
    http://www.student.nada.kth.se/~d95-mih/icq/
    they didn't work for me so I changed them
    to values that did work.
    *********************************/
   /*#define LOGIN_X1_DEF 0x00000078 */
   #define LOGIN_X1_DEF 0x00000098
/*   #define LOGIN_X2_DEF 0x04*/
   #define LOGIN_X2_DEF 0x06
   /*#define LOGIN_X3_DEF 0x00000002*/
   #define LOGIN_X3_DEF 0x00000003
   /*#define LOGIN_X4_DEF 0x00000000*/
   #define LOGIN_X4_DEF 0x00000000
   /*#define LOGIN_X5_DEF 0x00780008*/
   #define LOGIN_X5_DEF 0x00980000

#else /* ICQ_VER */

   typedef struct
   {
      BYTE port[4];
      BYTE len[2];
   } login_1, *LOGIN_1_PTR;

   typedef struct
   {
      BYTE X1[4];
      BYTE ip[4];
      BYTE  X2[1];
      BYTE  status[4];
      BYTE X3[4];
      BYTE seq[2];
      BYTE  X4[4];
      BYTE X5[4];
   } login_2, *LOGIN_2_PTR;

   /* those behind the // are for the spec on
    http://www.student.nada.kth.se/~d95-mih/icq/
    they didn't work for me so I changed them
    to values that did work.
    *********************************/
   /*#define LOGIN_X1_DEF 0x00000078 */
   #define LOGIN_X1_DEF 0x00040072
   #define LOGIN_X2_DEF 0x06
   /*#define LOGIN_X3_DEF 0x00000002*/
   #define LOGIN_X3_DEF 0x00000003
   /*#define LOGIN_X4_DEF 0x00000000*/
   #define LOGIN_X4_DEF 0x00000000
   /*#define LOGIN_X5_DEF 0x00780008*/
   #define LOGIN_X5_DEF 0x00720004

#endif /* ICQ_VER */

typedef struct
{
   BYTE   uin[4];
   BYTE year[2];
   BYTE  month;
   BYTE  day;
   BYTE  hour;
   BYTE  minute;
   BYTE type[2];
   BYTE len[2];
} RECV_MESSAGE, *RECV_MESSAGE_PTR;

typedef struct
{
   BYTE uin[4];
   BYTE type[2]; 
   BYTE len[2];
} SIMPLE_MESSAGE, *SIMPLE_MESSAGE_PTR;

typedef struct
{
   DWORD uin;
   DWORD status;
   DWORD last_time; /* last time online or when came online */
   BYTE current_ip[4];
   DWORD port;
   BOOL invis_list;
   BOOL vis_list;
   BOOL not_in_list;
   SOK_T sok;
   char nick[20];
} Contact_Member, *CONTACT_PTR;

typedef struct
{
   char *nick;
   char *first;
   char *last;
   char *email;
   BOOL auth;
} USER_INFO_STRUCT, *USER_INFO_PTR;

typedef struct
{
   char *nick;
   char *first;
   char *last;
   char *email;
   char *email2;
   char *email3;
   char *city;
   char *state;
   char *phone;
   char *fax;
   char *street;
   char *cellular;
   DWORD zip;
   WORD country;
   BYTE c_status;
   BOOL hide_email;
} MORE_INFO_STRUCT, *MORE_INFO_PTR;

void Keep_Alive( int sok );
void snd_got_messages( int sok );
void snd_contact_list( int sok );
void snd_invis_list( int sok );
void snd_vis_list( int sok );
void snd_login_1( int sok );
void Status_Update( int sok, BYTE * pak );
void Login( int sok, int UIN, char *pass, int ip, int port, DWORD status );
void ack_srv( SOK_T sok, DWORD seq );
void User_Offline( int sok, BYTE * pak );
int Save_RC();
void User_Online( int sok, BYTE * pak );
void M_fdprint( FD_T fd, char *str, ... );
int M_fdnreadln( FD_T fd, char *buf, size_t len );
char *Convert_Status_2_Str( int status );
void Print_Status( DWORD new_status );
void Get_Input( SOK_T sok );
void Quit_ICQ( SOK_T sok );
void icq_sendmsg( SOK_T sok, DWORD uin, char *text, DWORD msg_type);
void Recv_Message( SOK_T sok, BYTE * pak );
int Print_UIN_Name( DWORD uin );
void icq_change_status( SOK_T sok, DWORD status );
DWORD nick2uin( char * nick );
S_DWORD Echo_On( void );
S_DWORD Echo_Off( void );
void send_info_req( SOK_T sok, DWORD uin );
void Print_IP( DWORD uin );
void Display_Info_Reply( int sok, BYTE * pak );
WORD Chars_2_Word( unsigned char *buf );
DWORD Chars_2_DW( unsigned char *buf );
DWORD Get_Port( DWORD uin );
void Show_Quick_Status( void );
void icq_sendauthmsg( SOK_T sok, DWORD uin);
void Do_Msg( SOK_T sok, DWORD type, WORD len, char * data, DWORD uin );
void DW_2_Chars( unsigned char *buf, DWORD num );
void Word_2_Chars( unsigned char *buf, WORD num );
void Prompt( void );
void Time_Stamp( void );
void Add_User( SOK_T sok, DWORD uin, char *name );
void start_search( SOK_T sok, char *email, char *nick, char* first, char* last );
void Display_Search_Reply( int sok, BYTE * pak );
/*int log_event(char    *desc,char    *msg,DWORD   uin);*/
int log_event( int type, char *str, ... );
void rus_conv(char to[4],char *t_in);
void clrscr(void);
void reg_new_user( SOK_T sok, char *pass);
void send_ext_info_req( SOK_T sok, DWORD uin );
void Display_Ext_Info_Reply( int sok, BYTE * pak );
const char *Get_Country_Name( int code );
int Connect_Remote( char *hostname, int port, FD_T aux );
void Update_User_Info( SOK_T sok, USER_INFO_PTR user);
size_t SOCKWRITE( SOK_T sok, void * ptr, size_t len );
size_t SOCKREAD( SOK_T sok, void * ptr, size_t len );
void Hex_Dump( void *buffer, size_t len );
void icq_sendurl( SOK_T sok, DWORD uin, char *description, char *url );
void M_print( char *str, ... );
void Server_Response( SOK_T sok, BYTE *data, DWORD len, WORD cmd, WORD ver, DWORD seq, DWORD uin );
void icq_rand_user_req( SOK_T sok, DWORD group );
void icq_rand_set( SOK_T sok, DWORD group );
void Display_Rand_User( SOK_T sok, BYTE *data, DWORD len );
void Initialize_Msg_Queue( void );
void Do_Resend( SOK_T sok );
void Print_CMD( WORD cmd );
void Change_Password( SOK_T sok, char *pass );
void Meta_User( SOK_T sok, BYTE *data, DWORD len, DWORD uin );
void Kill_Prompt( void );
void Soft_Prompt( void );
char *UIN2nick( DWORD uin);
void Init_New_User( void );
CONTACT_PTR UIN2Contact( DWORD uin );
#ifdef UNIX 
void Get_Unix_Config_Info( void );
#endif

typedef struct {
	int   ready;
	DWORD uin;
	char  nickname[100];
	char  firstname[100];
} Human;

extern Contact_Member Contacts[ 100 ]; /* no more than 100 contacts max */
extern int Num_Contacts;
extern DWORD UIN; /* current User Id Number */
extern BOOL Contact_List;
extern WORD last_cmd[ 1024 ]; /* command issued for the first 1024 SEQ #'s */
/******************** should use & 0x3ff on all references to this */
extern WORD seq_num;  /* current sequence number */
extern DWORD our_ip;
extern DWORD our_port; /* the port to make tcp connections on */
extern BOOL Quit;
extern BOOL Verbose;
extern BYTE Sound;
extern DWORD Current_Status;
extern DWORD BotMode;
extern DWORD last_recv_uin;
extern Human Last_Probed_Rand_User;
extern char passwd[100];
extern char server[100];
extern DWORD remote_port;
extern DWORD set_status;
extern BOOL auto_resp;
extern char auto_rep_str_na[450];
extern char auto_rep_str_away[450];
extern char auto_rep_str_occ[450];
extern char auto_rep_str_inv[450];
extern char auto_rep_str_dnd[450];
extern BYTE Sound_Str[150];
extern BOOL Done_Login;
extern char message_cmd[16];
extern char info_cmd[16];
extern char quit_cmd[16];
extern char reply_cmd[16];
extern char again_cmd[16];
extern char add_cmd[16];

extern char list_cmd[16];
extern char away_cmd[16];
extern char na_cmd[16];
extern char dnd_cmd[16];
extern char online_cmd[16];
extern char occ_cmd[16];
extern char ffc_cmd[16];
extern char inv_cmd[16];
extern char status_cmd[16];
extern char auth_cmd[16];
extern char auto_cmd[16];
extern char search_cmd[16];
extern char save_cmd[16];
extern char change_cmd[16];
extern char alter_cmd[16];
extern char msga_cmd[16];
extern char url_cmd[16];
extern char sound_cmd[16];
extern char color_cmd[16];
extern char rand_cmd[16];
extern char update_cmd[16];

extern BOOL Russian;
extern BOOL Logging;
extern BOOL Color;

extern unsigned int next_resend;
extern DWORD our_session;
		
#define LOG_MESS 1
#define LOG_AUTO_MESS 2
#define LOG_ONLINE 3
