PATH_SRC   := src
PATH_BUILD := build
PATH_BIN   := $(PATH_BUILD)/bin
PATH_OBJ   := $(PATH_BUILD)/obj
PATH_DEP   := $(PATH_OBJ)/__dep__

CC        = g++
CXXFLAGS += -Wall -pedantic -g
CC        = $(CXX) $(CXXFLAGS)
CPPFLAGS += -std=c++17

MKDIR  = mkdir -p
RM     = rm -f
RMDIR  = rm -rf
COPY   = cp
MOVE   = mv -f
RENAME = mv -f

NULL_DEVICE = /dev/null

