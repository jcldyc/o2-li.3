CC = gcc -std=gnu99 -lpthread
SRCS = $(wildcard *.c)
PROGS = $(patsubst %.c,%,$(SRCS))
all: $(PROGS)
%: %.c
	$(CC) $(CFLAGS)  -o $@ $<
