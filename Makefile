CC             = gcc
CFLAGS         = -g -O3 -Wall -pedantic -pedantic-errors
SRCS           = myscd.c checker.c responder.c daemon.c utils.c iniparser/iniparser.c iniparser/dictionary.c iniparser/strlib.c
OBJS           = $(SRCS:.c=.o)
LDFLAGS        = -lpthread -lmysqlclient

all: myscd

myscd: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o myscd

clean:
	rm -f *.o
	rm -f iniparser/*.o
	rm -f *.a
	rm -f *.so
	rm -f myscd
