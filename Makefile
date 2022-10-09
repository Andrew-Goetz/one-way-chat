CC:= gcc
CFLAGS:= -O2 -g

all:		sender receiver

sender: 	sender.c

receiver: 	receiver.c
