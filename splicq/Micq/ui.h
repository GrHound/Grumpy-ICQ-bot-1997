static DWORD last_uin=0;
static DWORD multi_uin;
static int status = 0;
static void Show_Status( void );
static DWORD uin;

#define UIN_DELIMS ":|/" /* add space if you don't want your nick names to have spaces */
#define END_MSG_STR "."
#define CANCEL_MSG_STR "#"
#define W_SEPERATOR MESSCOL "============================================" NOCOL "\n"
#define NEW_NICK 10
#define NEW_FIRST 11
#define NEW_LAST 12
#define NEW_EMAIL 13
#define NEW_AUTH 14

#define SRCH_START 15
#define SRCH_EMAIL 16
#define SRCH_NICK 17
#define SRCH_FIRST 18
#define SRCH_LAST 19


#define CHANGE_STATUS( a )  icq_change_status( sok, a ); \
                           Print_Status( Current_Status ); \
                            printf( "\n" );


static void Change_Function( SOK_T sok );
static void Help_Function( void );
static void Info_Function( SOK_T sok );
static void Auto_Function( SOK_T sok );
static void Alter_Function( void );
static void Message_Function( SOK_T sok );
static void Reply_Function( SOK_T sok );
static void Again_Function( SOK_T sok );
static void Verbose_Function( void );
static void Random_Function( SOK_T sok );
static void Random_Set_Function( SOK_T sok );
