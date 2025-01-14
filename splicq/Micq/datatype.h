#ifndef __MATT_DATATYPE__
#define __MATT_DATATYPE__
#ifdef UNIX
   #include <unistd.h>
#endif

typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef signed long S_DWORD;
typedef signed short S_WORD;
typedef signed char S_BYTE;
typedef signed long INT32;
typedef signed short INT16;
typedef signed char INT8;
typedef unsigned long U_INT32;
typedef unsigned short U_INT16;
typedef unsigned char U_INT8;

#ifdef _WIN32
  typedef int FD_T;
  typedef int SOK_T;
  typedef unsigned int ssize_t;
  typedef int BOOL;
  #define sockread(s,p,l) recv(s,(char *) p,l,0)

/* use SOCKWRITE !!!!! */
  #define sockwrite(s,p,l) send(s,(char *) p,l,0)
  #define SOCKCLOSE( s ) closesocket(s)
  #define strcasecmp(s,s1)  stricmp(s,s1)
  #define strncasecmp(s,s1,l)  strnicmp(s,s1,l)
  #define Get_Config_Info() Get_Unix_Config_Info()
#else
  typedef unsigned char BOOL;
#endif

#ifdef UNIX
  #define sockread(s,p,l) read(s,p,l)

/* use SOCKWRITE !!!!! */
  #define sockwrite(s,p,l) write(s,p,l)
  #define SOCKCLOSE( s ) close(s)
  #define Get_Config_Info() Get_Unix_Config_Info()
  typedef int FD_T;
  typedef int SOK_T;
#endif /* UNIX */

#ifndef TRUE
  #define TRUE 1
#endif

#ifndef FALSE
  #define FALSE 0
#endif

#endif /* Matt's datatype */
