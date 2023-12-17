CXXFLAGS += -Wall -pedantic -pthread
CC        = $(CXX) $(CXXFLAGS)
CPPFLAGS += -std=c++17
CC       += $(CPPFLAGS)

MKDIR  = mkdir -p
RM     = rm -f
RMDIR  = rm -rf
COPY   = cp
MOVE   = mv -f
RENAME = mv -f
NULL_DEVICE = /dev/null
