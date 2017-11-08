#
# Makefile for demo with hhlib.so


# Paths

LPATH = /usr/local/lib64/hh400/

# Flags

CC = gcc

WARN = -Wall -Wno-format
COPTS = -g -O

CFLAGS = $(WARN) $(COPTS)

# Rules

%.o: %.c
	$(CC) -c $(CFLAGS) -I$(LPATH) -o $@ $<

# Variables

BINS = histomode
SRCS = histomode.c cmdline.c
OBJS = $(SRCS:%.c=%.o)

# Main target

all: $(BINS)

# Dependencies

histomode: cmdline.c histomode.o cmdline.o 
	$(CC) $(OBJS) $(LPATH)hhlib.so -o $@

cmdline.c: histomode.cmdline
	gengetopt --input=$< --include-getopt

# Misc

clean:
	rm -f *.o *~ ~* *.bck core cmdline.c cmdline.h
	rm -f $(BINS)

