# the compiler: gcc for C program, define as g++ for C++
CC  = gcc
CPP = g++
MAKEFLAGS = --silent

PRJNAME=Qme

# PREFIX is environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
    PREFIX := /var/spool/$(PRJNAME)
endif

# special include directories
INCLUDE = -I.

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
# CFLAGS  = -g -Wall -Ofast -g -Warray-bounds -fsanitize=address
CFLAGS  = -Ofast


DEPS = main.o Task.o List.o Queue.o

all: $(PRJNAME)

# verbose: CFLAGS += -DVERBOSE -g -Warray-bounds -DDEBUG -fsanitize=address # -dM -E
# verbose: all

# debug: CFLAGS += -DDEBUG -g -Warray-bounds -fsanitize=address # -dM -E
# debug: all


$(PRJNAME): $(DEPS)
	$(CC) $(CFLAGS) -o $(PRJNAME) $+ -lm

%.o: %.c
	$(CC) $(CFLAGS) $< -c


.PHONY: all

clean:
	$(RM) $(PRJNAME) *.o *~

install:
	install -m 0777 -d $(PREFIX)
	install -m 0777 -d $(PREFIX)/info
	install -m 0777 -d $(PREFIX)/init
	install -m 0777 -d $(PREFIX)/queued
	install -m 0777 -d $(PREFIX)/running
	install -m 0777 -d $(PREFIX)/finished
	install -m 0777 taskid $(PREFIX)/info/
	install -m 0777 version $(PREFIX)/info/
	install Qme_init /etc/init.d/Qme
	install $(PRJNAME) /usr/bin

	echo "Start init process:"
	echo "	# /etc/init.d/Qme start"
	echo "Add Qme to runlevel:"
	echo "	# rc-update add Qme"
