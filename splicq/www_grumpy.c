/* WWW wrap-around for Grumpy the chatbot 
   Lambert Schomaker */

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "sock_comm.h"

static FILE *log; /* used to log incoming client information */

#define FBUFSIZ 2048

#define me_URL "http://ai17872.ai.rug.nl:8080"

static char old_URL[256],new_URL[256],catcher_log[256],image_URL[256];
static int http_port;

#define NMED 100000
#define NBIG 1000000
static char largebuf[NBIG],mquestion[NMED],question[NMED],answer[NBIG],oldanswer[NBIG];


static char *header = "<html>\n\
<head>\n<title>GRUMPY - your unfriendly chatbot</title></head>\n<body>\n\
<h1>GRUMPY - your unfriendly chatbot</h1>\n\
<hr><p>\n\
<kbd>%s</kbd><br><font size=+2>\n\
<kbd>Grumpy: %s</kbd><p>\n\
<kbd>You: %s</kbd><p>\n\
<kbd>Grumpy: %s</kbd>\n\
<p>\n\
<kbd>You:</kbd></font> &nbsp;\n\
<form id=\"human\" name=\"human\" action=\"http://ai17872.ai.rug.nl:8080\" method=get>\n\
<input type=hidden name=cmd value=\"humansays\">\n\
<input type=text name=txt size=50 value=\"\"></textarea>\n\
<label for=\"txt\"> &nbsp; (please type English text for Grumpy)</label>\n\
</form>\n\
<script>\n\
document.human.txt.focus();\n\
</script>\n\
<a href=%s/?cmd=restart&txt=Hi+Grumpy><pre>Start all over</pre></a><br>\n\
<hr>\n";

#define tail "</BODY>\n</html>\n"

#define DIFFERENT_ADDRESS      -1
#define SAME_ADDRESS            0
#define SAME_INSTITUTE          1
#define SAME_ORGANIZATION       2
#define SAME_COUNTRY            3

/* url/http manglers */

void plustospace(char *str) {
    register int x;

    for(x=0;str[x];x++) if(str[x] == '+') str[x] = ' ';
}

char x2c(char *what) {
    register char digit;

    digit = ((what[0] >= 'A') ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
}

void unescape_url(char *url) {
    register int x,y;

    for(x=0;url[x];x++)
        if(url[x] == '%')
            url[x+1] = x2c(&url[x+1]);

    for(x=0,y=0;url[y];++x,++y) {
        if((url[x] = url[y]) == '%') {
            url[x] = url[y+1];
            y+=2;
        }
    }
    url[x] = '\0';
}

char *cleanup(char *raw)
{
    char *t;
    static char cln[FBUFSIZ];
    
    t = (char *) malloc(sizeof(char) * (strlen(raw)+1));
    if(t == NULL) {
      fprintf(stderr,"malloc() error in cleanup()");
      exit(1);
    }
    strcpy(t, raw);
    plustospace(t);
    unescape_url(t);
    strcpy(cln,t);
    free(t);
    return(cln);
}


int same_address (char *a1, char *a2)
{
	return (strcmp(a1,a2)==0);
}


int same_institute (char *a1, char *a2)
{
	char *p1,*p2;

	if ((p1=strchr(a1,'.'))==NULL)
		return 0;
	if ((p2=strchr(a2,'.'))==NULL)
		return 0;
	return (strcmp(p1,p2)==0);
}

int same_organization (char *a1, char *a2)
{
	char *p1,*p2;

	if ((p1=strchr(a1,'.'))==NULL)
		return 0;
	if ((p1=strchr(p1+1,'.'))==NULL)
		return 0;
	if ((p2=strchr(a2,'.'))==NULL)
		return 0;
	if ((p2=strchr(p2+1,'.'))==NULL)
		return 0;
	return (strcmp(p1,p2)==0);
}

int same_country (char *a1, char *a2)
{
	char *p1,*p2;

	if ((p1=strchr(a1,'.'))==NULL)
		return 0;
	if ((p1=strchr(p1+1,'.'))==NULL)
		return 0;
	if ((p1=strchr(p1+1,'.'))==NULL)
		return 0;
	if ((p2=strchr(a2,'.'))==NULL)
		return 0;
	if ((p2=strchr(p2+1,'.'))==NULL)
		return 0;
	if ((p2=strchr(p2+1,'.'))==NULL)
		return 0;
	return (strcmp(p1,p2)==0);
}

int compare_hostnames (char *h1, char *h2)
{
	if (same_address(h1,h2))
		return SAME_ADDRESS;
	if (same_institute(h1,h2))
		return SAME_INSTITUTE;
	if (same_organization(h1,h2))
		return SAME_ORGANIZATION;
	if (same_country(h1,h2))
		return SAME_COUNTRY;
	return DIFFERENT_ADDRESS;
}

void print_iaddr (char **alist)
{
	unsigned char apie[4];

	apie[0] = alist[0][0];
	apie[1] = alist[0][1];
	apie[2] = alist[0][2];
	apie[3] = alist[0][3];
	fprintf (stderr,"%u.%u.%u.%u\n",apie[0],apie[1],apie[2],apie[3]);
}

void report_about_direct_path (char *buf, char *newpath, char *rem_host)
{
	char locbuf[FBUFSIZ];

	sprintf (locbuf,"You entered this URL by typing the location at your browser running on %s",rem_host);
	strcat(buf,locbuf);
	fprintf (log,"got a direct path <pre>%s</pre> from browser %s\n",newpath,rem_host);
	fflush(log);
}

void give_moved_message (int fd, char *newpath, char *referer, char *loc_host)
{
	char rem_host[512];
#ifdef LOCAL_STRINGS
	char largebuf[NBIG];
	char mquestion[NMED], question[NMED], answer[NBIG];
#endif
	char *ptr;

	get_remote_host(fd,rem_host);
	if ((ptr=strchr(newpath,' '))!=NULL) {
		*ptr = '\0';
	}
	ptr = strstr(newpath,"&txt=");
	if(ptr != NULL) {
	   strcpy(question,cleanup(ptr+5));
	   strcpy(newpath," "); /* comment if debug needed on url */
        }
	
        strcpy(mquestion,question);
        
        chatty("human-X", mquestion, answer);
        if(answer[0] < 32) {
           strcpy(answer,"?");
        }

	sprintf (largebuf,header,newpath,oldanswer,question,answer,me_URL,me_URL);
	strcpy(oldanswer, answer);
	if (strcmp(referer,"????????")==0) {
		report_about_direct_path(largebuf,newpath,rem_host);
	}

	sprintf(rem_host,"<p><hr> \
<img align=left src=http://%s/Grumpy/blueface.gif> This page is \
generated by the <b>Grumpy www agent</b>. For information about this program, \
please contact <a \
href=mailto:schomaker@ai.rug.nl>schomaker@ai.rug.nl</a>.\n",image_URL);
	strcat(largebuf,rem_host);
	strcat(largebuf,tail);
	full_write(fd,largebuf,strlen(largebuf)+1);
}


int get_request_line(int fd, char *line)
{
	char *ptr,c='a';
	int i = 0;

	while (full_read(fd,&c,sizeof(char))==1) {
		if (c=='\n')
			break;
		line[i++] = c;
	}
	line[i] = '\0';
	if ((ptr=strchr(line,'\r'))!=NULL)
		ptr[0] = '\0';
	return (i!=1);
}

void empty_http_requests (int fd, char *newpath, char *referer,char *loc_host)
{
	char request_line[2048];

	strcpy (referer,"????????");
	strcpy (loc_host,"????????");
	while (get_request_line(fd,request_line)) {
		fprintf (stderr,"request is (%s)\n",request_line);
		if (strncmp(request_line,"GET /",5) == 0) {
			strcpy(newpath,&request_line[5]);
		}
		if (strncmp(request_line,"Host:",5)==0) {
			strcpy(loc_host,&request_line[5]);
		}
		if (strncmp(request_line,"Referer:",8)==0) {
			strcpy(referer,&request_line[8]);
		}
	}
fprintf (stderr,"\n---- end request ---- \n");
}

/* ***************** how to parse the command line ************** */

static void help_catcher (void)
{
	fprintf (stderr,"use: 'www_catcher [options]' or www_catcher -help\n");
	fprintf (stderr,"options: -o old_URL\n");
	fprintf (stderr,"         -n new_URL\n");
	fprintf (stderr,"         -l logfile\n");
	fprintf (stderr,"         -p portnr\n");
	fprintf (stderr,"example: www_catcher -o www.nici.kun.nl -n www.cogsci.kun.nl -l new.log -p 1234\n");
}

int init_catcher (int argc, char *argv[])
{
	int i,port;

	/* check if OLD_URL, NEW_URL, HTTP_PORT, and CATCHER_LOG are defined */
#ifndef OLD_URL
	fprintf (stderr,"catcher was not compiled with -DOLD_URL=$(OLD_URL)!\n");
	fprintf (stderr,"add a line to the Makefile like:\n");
	fprintf (stderr,"OLD_URL = '\"www.nici.kun.nl\"'\n");
	exit(1);
#endif
#ifndef NEW_URL
	fprintf (stderr,"catcher was not compiled with -DNEW_URL=$(NEW_URL)!\n");
	fprintf (stderr,"add a line to the Makefile like:\n");
	fprintf (stderr,"NEW_URL = '\"www.cogsci.kun.nl\"'\n");
	exit(1);
#endif
#ifndef HTTP_PORT
	fprintf (stderr,"catcher was not compiled with -DHTTP_PORT=$(HTTP_PORT)!\n");
	fprintf (stderr,"add a line to the Makefile like:\n");
	fprintf (stderr,"HTTP_PORT = 80\n");
	exit(1);
#endif
#ifndef CATCHER_LOG
	fprintf (stderr,"catcher was not compiled with -DCATCHER_LOG=$(CATCHER_LOG)!\n");
	fprintf (stderr,"add a line to the Makefile like:\n");
	fprintf (stderr,"CATCHER_LOG = '\"catcher.log\"'\n");
	exit(1);
#endif

	/* set default values */
	strcpy(old_URL,OLD_URL);
	strcpy(new_URL,NEW_URL);
	strcpy(catcher_log,CATCHER_LOG);
	port = HTTP_PORT;
	for (i=1;i<argc;i++) {
		if (strcmp(argv[i],"-o")==0) {
			strcpy(old_URL,argv[i+1]);
			i++;
			continue;
		}
		if (strcmp(argv[i],"-n")==0) {
			strcpy(new_URL,argv[i+1]);
			i++;
			continue;
		}
		if (strcmp(argv[i],"-l")==0) {
			strcpy(catcher_log,argv[i+1]);
			i++;
			continue;
		}
		if (strcmp(argv[i],"-p")==0) {
			port = atoi(argv[i+1]);
			i++;
			continue;
		}
		fprintf (stderr,"unknown option '%s'!\n",argv[i]);
		help_catcher();
		exit(1);
	}
	if ((log=fopen(catcher_log,"a+"))==NULL) {
		fprintf (stderr,"unable to open logfile %s!\n",argv[2]);
		exit(1);
	}
	strcpy(image_URL,old_URL);
	return port;
}


int main(int argc, char *argv[])
{
	int http_socket,fd;
	char referer[FBUFSIZ],loc_host[512],newpath[FBUFSIZ];
	
	http_port = init_catcher(argc,argv);
	
        strcpy(oldanswer,"(sleeping)");

	/* the server creates a http socket on port 'http_port' */

	http_socket = create_server_connection ((unsigned short)http_port);
	fprintf (stderr,"SERVER WAITING FOR CONNECTIONS\n");
	fprintf (stderr,"http_socket = %d\n",http_socket);
	if (http_socket<=0)
		exit(1);

	/* The server listens to any incoming clients     */
	/* it empties the requests sent by a new client ; */
	/* it reports the 'WE ARE MOVED' message;         */
	/* and shuts the connection down.                 */

	while(1) {
		if ((fd=accept_new_client(http_socket,-1))>=0) {
			empty_http_requests (fd,newpath,referer,loc_host);
			give_moved_message(fd,newpath,referer,loc_host);
			close_connection(fd);
		}
	}
	return 0;
}
