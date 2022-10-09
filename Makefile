CC:= gcc
CFLAGS:= -O2 -g

all:		sender receiver

sender: 	sender.c

receiver: 	receiver.c

.PHONY:		clean
clean:
	rm -f *.o sender receiver
