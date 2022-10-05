CC = gcc # Flag for implicit rules
CFLAGS = -g # Flag for implicit rules. Turn on debug info
LDLIBS = -lacl # Flag for implicit rules. Lik acl library

INCLUDE_PATH := ./include

INCLUDE_FILE := $(shell find $(INCLUDE_PATH) -name '*.c')

INCLUDE_OBJS := $(INCLUDE_FILE:%.c=%.o)

ls: ls.o ${INCLUDE_OBJS}

clean:
	rm -f ls.o ls ${INCLUDE_OBJS}