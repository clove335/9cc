CC=gcc
CFLAGS=-Wall -std=c11 -g -static -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
				$(CC) -o 9cc $(SRCS) $(LDFLAGS)

$(OBJS): 9cc.h

.PHONY: test clean

test: 9cc
				./9cc -test
				./test.sh

clean:
			rm -f 9cc *.o *~ tmp*
