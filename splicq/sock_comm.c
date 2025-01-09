#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "sock_comm.h"

#ifdef NO_HTTP
static int first_read (int, char *, int);
#endif

/* the global communication and process datastructures */


int setup_localconnection (int *sv)
{
	if (socketpair (PF_UNIX,SOCK_STREAM,0,sv)!=0) {
		perror ("socketpair: ");
		return -1;
	}
	return SOCK_OK;
}

#ifdef _HP_
#define FD_MASK_DECLARE int fdmask[8]
#define MYMASK(fd,fdmask) {\
	memset (fdmask,0,8*sizeof(int));\
	fdmask[fd/(sizeof(int)*8)] |= (1 << (fd % 8));\
}
#define SELECT_ON_MASK_FD(fd,fdmask) {\
	MYMASK(fd,fdmask);\
	if (tv_sec<0) {\
		select (FD_SETSIZE,fdmask,NULL,NULL,forever);\
	}\
	else {\
		if (select (FD_SETSIZE,fdmask,NULL,NULL,&timeout)<=0)  {\
			return TIMEOUT_OCCURRED;\
		}\
	}\
	if ((fdmask[fd/(sizeof(int)*8)] & (1 << (fd % 8)))==0) {\
		perror ("fd-bit not set");\
		return FDSET_ERROR;\
	}\
}
#else
#define FD_MASK_DECLARE fd_set fdmask
#define MYMASK(fd,fdmask) {\
	FD_ZERO (&fdmask);\
	FD_SET (fd,&fdmask);\
}
#define SELECT_ON_MASK_FD(fd,fdmask) {\
	MYMASK(fd,fdmask);\
	if (tv_sec<0) {\
		select (FD_SETSIZE,&fdmask,NULL,NULL,forever);\
	}\
	else {\
		if (select (FD_SETSIZE,&fdmask,NULL,NULL,&timeout)<=0)  {\
			return TIMEOUT_OCCURRED;\
		}\
	}\
	if (FD_ISSET(fd,&fdmask)==0) {\
		perror ("fd-bit not set");\
		return FDSET_ERROR;\
	}\
}
#endif

int wait_for_request (int fd, int tv_sec)
{
	FD_MASK_DECLARE;
	struct timeval timeout;
	struct timeval *forever = NULL;

	timeout.tv_sec = tv_sec;
	timeout.tv_usec = 0;
	SELECT_ON_MASK_FD(fd,fdmask);
	return 1;
}

void get_remote_host (int s, char *hname) 
{
	struct sockaddr addr;
	int len;
	struct in_addr *iaddr;
	struct hostent *hptr;

	len = sizeof(struct sockaddr);
    
	if ((getpeername(s, &addr, &len)) < 0) {
		fprintf (stderr,"UNKNOWN REMOTE HOST\n");
		return;
	}

	iaddr = &(((struct sockaddr_in *)&addr)->sin_addr);
	hptr = gethostbyaddr((char *)iaddr, sizeof(struct in_addr), AF_INET);
	fprintf (stderr,"remote host is %s\n",hptr->h_name);
	strcpy(hname,hptr->h_name);
}

int new_client_arrived (int server_sock, int tv_sec)
{
	FD_MASK_DECLARE;
	struct timeval timeout;
	struct timeval *forever = NULL;

	int fd = server_sock;
	timeout.tv_sec = tv_sec;
	timeout.tv_usec = 0;
	SELECT_ON_MASK_FD(fd,fdmask);
	return 1;
}

int accept_the_newly_arrived (int server_sock)
{
	int from_length,new_sock;
	struct sockaddr_in from;

	from_length = sizeof(from);
	if ((new_sock = accept (server_sock,(struct sockaddr *)&from,&from_length))<0) {
		perror ("accept server");
		return (ACCEPT);
	}
	fcntl (new_sock,F_SETFD,O_NONBLOCK);
	return new_sock;
}

int accept_new_client (int server_sock, int tv_sec)
{
	int from_length,new_sock;
	struct sockaddr_in from;
	FD_MASK_DECLARE;
	struct timeval timeout;
	struct timeval *forever = NULL;

	int fd = server_sock;
	timeout.tv_sec = tv_sec;
	timeout.tv_usec = 0;
	SELECT_ON_MASK_FD(fd,fdmask);
	from_length = sizeof(from);
	if ((new_sock = accept (server_sock,(struct sockaddr *)&from,&from_length))<0) {
		perror ("accept server");
		return (ACCEPT);
	}
	fcntl (new_sock,F_SETFD,O_NONBLOCK);
#ifdef NO_HTTP
	full_write (new_sock,(char *) &from_length, sizeof(int));
	if (first_read (new_sock,(char *) &from_length, sizeof(int))==CONNECTION_LOST)
		return CONNECTION_LOST;
#endif
	return new_sock;
}


int create_server_connection (unsigned short hard_address)
{
	int new_socket;
	struct sockaddr_in addr;

	if ((new_socket = socket (AF_INET,SOCK_STREAM,0))<0) {
		perror ("socket server");
		return (SOCKET);
	}
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(hard_address);
	
	if (bind (new_socket,(struct sockaddr *)&addr,sizeof(addr))<0) {
		perror ("bind server");
		return (BIND);
	}
	
	listen (new_socket,5); /* 5 = pending connections */
	return new_socket;
}
	

int setup_server_connection (int *server_sock, unsigned short hard_address, int timeout)
{
	int new_socket;
	struct sockaddr_in addr;

	if ((*server_sock = socket (AF_INET,SOCK_STREAM,0))<0) {
		perror ("socket server");
		return (SOCKET);
	}
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(hard_address);
	
	if (bind (*server_sock,(struct sockaddr *)&addr,sizeof(addr))<0) {
		perror ("bind server");
		return (BIND);
	}
	
	listen (*server_sock,5); /* 5 = pending connections */

	new_socket = accept_new_client (*server_sock,timeout);
	return new_socket;
}

int setup_client_connection (char *hostname, unsigned short hard_address)
{
	int s;
	struct sockaddr_in addr;
	char newhostname[100];
	
	struct hostent *hp;
	
	if (hostname==NULL) {
		printf ("enter hostname: ");
		fflush(stdout);
		scanf ("%s",newhostname);
		printf ("\nenter portnr: ");
		fflush (stdout);
		scanf ("%hd",&hard_address);
	}
	else
		strcpy (newhostname,hostname);
	hp = gethostbyname (newhostname);
	if (hp == NULL)
		hp = gethostbyaddr (newhostname,sizeof(struct in_addr),AF_INET);
	if (hp == NULL) {
		 printf("%s: unknown host\n",newhostname);
		 exit(1);
	}
	memcpy((char *)&addr.sin_addr.s_addr, (char *)hp->h_addr, hp->h_length);
	s = socket (AF_INET,SOCK_STREAM,0);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(hard_address);
	
	if (connect (s,(struct sockaddr *)&addr,sizeof(addr))<0) {
		perror ("connect client");
		exit(1);
	}
#ifdef NO_HTTP
int i;
	full_read (s,(char *)&i,sizeof(int));
	full_write (s,(char *)&i,sizeof(int));
#endif
	return s;
}

void close_connection (int fd)
{
	close(fd);
	shutdown(fd,2);
}

/* BASIC COMMUNICATION PRIMITIVES full_read, full_write */


int full_read (int fd, char * buf, int nbytes)
{
	int n,total,retry;
	n = retry = 0;
	total = nbytes;
	while (nbytes>0) {
		switch (n=read(fd,buf,nbytes)) {
			case 0:
				if (++retry<MAXRETRY) {
					fprintf (stderr,"read ZERO, retrying (RETRY=%d)\n",retry);
					break;
				}
				else {
					fprintf (stderr,"ASSUMING COMMUNICATION LOST, EXITING\n");
					return total - nbytes;
				}
			case -1:
				perror ("read error: ");
				return n;
				break;
			default:
				nbytes -= n;
				buf += n;
				retry = 0;
				break;
		}
	}
	return total-nbytes;
}

int full_write (int fd, char * buf, int nbytes)
{
	int n,total;
	n = total = 0;
	while (nbytes-total>0) {
		if ((n=write(fd,buf+total,nbytes-total))<0) {
			perror ("write error: ");
			return n;
		}
		total += n;
	}
	return total;
}
#ifdef NO_HTTP
static int first_read (int fd, char * buf, int nbytes)
{
	int n,total,retry;
	n = retry = 0;
	total = nbytes;
	while (nbytes>0) {
		switch (n=read(fd,buf,nbytes)) {
			case 0:
				if (++retry<MAXRETRY) {
					fprintf (stderr,"read ZERO, retrying (RETRY=%d)\n",retry);
					break;
				}
				else {
					fprintf (stderr,"ASSUMING COMMUNICATION LOST\n");
					return CONNECTION_LOST;
				}
			case -1:
				perror ("read error: ");
				return n;
				break;
			default:
				nbytes -= n;
				buf += n;
				retry = 0;
				break;
		}
	}
	return total-nbytes;
}
#endif
