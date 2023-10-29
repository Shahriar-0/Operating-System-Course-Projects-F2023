CC        = gcc
CFLAGS   += -Wall -pedantic
CC       += $(CFLAGS)
CPPFLAGS += -std=gnu99

MKDIR  = mkdir -p
RM     = rm -f
RMDIR  = rm -rf
COPY   = cp
MOVE   = mv -f
RENAME = mv -f
NULL_DEVICE = /dev/null
