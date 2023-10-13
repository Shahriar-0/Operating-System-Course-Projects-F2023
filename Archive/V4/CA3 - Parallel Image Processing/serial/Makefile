# ------------------------------------------------------------------
PATH_SRC     := src
PATH_BUILD   := build
PATH_INC     := $(PATH_SRC)/include
PATH_OBJ     := $(PATH_BUILD)/obj
# ------------------------------------------------------------------
MKDIR        = mkdir -p
RM           = rm -f
RMDIR        = rm -rf
# ------------------------------------------------------------------
CXX          = g++
CXXFLAGS     = -Wall -pedantic -std=c++11 -I$(PATH_INC)
CC           = $(CXX) $(CXXFLAGS)
# ------------------------------------------------------------------
OBJS         = main.o bmp24.o filter.o
EXE_MAIN     = ImageFilters.out
OUT_BMP      = output.bmp
# ------------------------------------------------------------------
vpath %.cpp $(PATH_SRC)
vpath %.hpp $(PATH_INC)
# ------------------------------------------------------------------
all: $(PATH_OBJ) $(EXE_MAIN)

$(EXE_MAIN): $(addprefix $(PATH_OBJ)/, $(OBJS))
	$(CC) -o $@ $^

$(PATH_OBJ)/main.o: main.cpp bmp24.hpp filter.hpp
	$(CC) -c -o $@ $<

$(PATH_OBJ)/bmp24.o: bmp24.cpp bmp24.hpp
	$(CC) -c -o $@ $<

$(PATH_OBJ)/filter.o: filter.cpp filter.hpp bmp24.hpp
	$(CC) -c -o $@ $<
# ------------------------------------------------------------------
$(PATH_OBJ):  ; $(MKDIR) $@
# ------------------------------------------------------------------
.PHONY: all \
		clean clean-obj clean-exe clean-img\
		run

ARGS ?=
run: ; ./$(EXE_MAIN) $(ARGS)

clean: clean-obj clean-exe clean-img
clean-obj: ; $(RMDIR) $(PATH_OBJ)/*
clean-exe: ; $(RM) $(EXE_MAIN)
clean-img: ; $(RM) $(OUT_BMP)
# ------------------------------------------------------------------