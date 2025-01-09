# Anyone who knows how to make Makefiles please help out.
# I have no clue but this appears to work fairly well
# be sure to use gmake
CC = gcc
CFLAGS = -O4 -Wall -pedantic -DUNIX -DANSI_COLOR 
CFLAGS += -DUSE_MREADLINE
CFLAGS += -DENGLISH_LANG
#CFLAGS += -DBRAZIL_LANG
#CFLAGS += -DRUSSIAN_LANG
# Uncomment the below line for humorous messages
#CFLAGS += -DFUNNY_MSGS 
# uncomment the line below for debuging info
#CFLAGS += -g
CLIBS = 
#uncomment the line below if your using Solaris
#CLIBS= -lnsl -lsocket
#uncomment the line below if your using MiNT
#CLIBS= -lsocket
#uncomment the line below if your using NeXT
#CFLAGS += -posix
OBJS =file_util.o icq_response.o micq.o msg_queue.o rus_conv.o sendmsg.o \
      server.o splotch.o calculator.o \
      ui.o util.o  util_ui.o mreadline.o
      
HEADERS = datatype.h micq.h ui.h msg_queue.h mreadline.h
#
all : micq

re : clean all

micq : ${OBJS} ${HEADERS} 
	${CC} ${CFLAGS} -lm -o micq ${OBJS} ${CLIBS}

${OBJS} : ${HEADERS}

clean :
	rm -f ${OBJS}

