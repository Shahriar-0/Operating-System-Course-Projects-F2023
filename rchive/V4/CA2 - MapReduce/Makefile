# ------------------------------------------------------------------
PATH_INCLUDE := include
PATH_SRC     := src
PATH_BIN     := bin
PATH_OBJ     := $(PATH_BIN)/obj
# ------------------------------------------------------------------
RM           = @rm -rf
MKDIR        = @mkdir -p
# ------------------------------------------------------------------
CXX          = g++
CXXFLAGS     = -std=c++11
CC           = $(CXX) $(CXXFLAGS)
# ------------------------------------------------------------------
NAMED_PIPES := position_country_pipe
OBJS         = $(PATH_OBJ)/main.o $(PATH_OBJ)/utils.o
POSITION     = position.out
COUNTRY      = country.out
CLUB         = club.out
OUT_EXE      = ClubsAgeStats.out
# ------------------------------------------------------------------
all: pre_build build final

pre_build:
	$(MKDIR) $(PATH_OBJ)

build: $(POSITION) $(COUNTRY) $(CLUB)

final: $(OBJS)
	$(CC) $(OBJS) -o $(OUT_EXE)
# ------------------------------------------------------------------
$(PATH_OBJ)/main.o: $(PATH_SRC)/main.cpp $(PATH_INCLUDE)/utils.h
	$(CC) -c $(PATH_SRC)/main.cpp -o $@

$(PATH_OBJ)/utils.o: $(PATH_SRC)/utils.cpp $(PATH_INCLUDE)/utils.h
	$(CC) -c $(PATH_SRC)/utils.cpp -o $@
# ------------------------------------------------------------------
$(POSITION): $(PATH_SRC)/position.cpp $(PATH_OBJ)/utils.o
	$(CC) $(PATH_SRC)/position.cpp $(PATH_OBJ)/utils.o -o $@

$(COUNTRY): $(PATH_SRC)/country.cpp $(PATH_OBJ)/utils.o
	$(CC) $(PATH_SRC)/country.cpp $(PATH_OBJ)/utils.o -o $@

$(CLUB): $(PATH_SRC)/club.cpp $(PATH_OBJ)/utils.o
	$(CC) $(PATH_SRC)/club.cpp $(PATH_OBJ)/utils.o -o $@
# ------------------------------------------------------------------
clean:
	$(RM) $(PATH_BIN) *.out $(NAMED_PIPES) $(OUT_EXE)
# ------------------------------------------------------------------