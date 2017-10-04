CC = gcc -std=gnu99
SRCS = $(wildcard *.c)
PROGS = $(patsubst %.c,%,$(SRCS))
all: $(PROGS)
%: %.c
	$(CC) $(CFLAGS)  -o $@ $<
	
clean:
    rm -f master master.o palin palin.o

