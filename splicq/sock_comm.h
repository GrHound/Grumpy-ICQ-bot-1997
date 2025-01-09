#ifndef SOCK_COMM
#define SOCK_COMM

#define server_ip "kunhp1.psych.kun.nl"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CONNECTION_TIMEOUT 25

#define MAXRETRY            5

#define SOCK_OK            1
#define SOCKET             -1
#define BIND               -2
#define ACCEPT             -3
#define UNKNOWN_HOST       -4
#define CONNECT            -5
#define TIMEOUT_OCCURRED   -6
#define FDSET_ERROR        -7
#define CONNECTION_LOST    -8

typedef unsigned char Byte;
typedef struct {
	int fd;
	int pid;
	char *name;
} ProcessorList;


extern void get_remote_host (int s, char *hname);

extern int setup_localconnection (int *);
extern int accept_new_client (int, int);
extern int accept_the_newly_arrived (int);
extern int new_client_arrived (int, int tv_sec);
extern int create_server_connection (unsigned short hard_address);
extern int setup_server_connection (int *, unsigned short, int);
extern int setup_client_connection (char *,unsigned short);
extern void close_connection (int);

extern int wait_for_request (int fd, int tv_sec);


extern int full_read (int, char *, int);
extern int full_write (int, char *, int);

#endif
