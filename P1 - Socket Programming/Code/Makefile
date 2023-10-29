PATH_SRC   := src
PATH_BUILD := build
PATH_BIN   := $(PATH_BUILD)/bin
PATH_OBJ   := $(PATH_BUILD)/obj
PATH_DEP   := $(PATH_OBJ)/__dep__

include common_vars.mk

OUT_EXE_CLIENT := main
OUT_EXE_SERVER := server

#----------------------------------------

VPATH = $(PATH_SRC)

FILES   = $(patsubst src/%, %, $(shell find $(PATH_SRC) -name "*.c" -type f))
FOLDERS = $(patsubst src/%, %, $(shell find $(PATH_SRC) -mindepth 1 -type d))

FILES_NOMAIN = $(filter-out main.c, $(FILES))

FILES_DEP = $(patsubst %, $(PATH_DEP)/%.d, $(basename $(FILES)))
FILES_OBJ = $(patsubst %, $(PATH_OBJ)/%.o, $(basename $(FILES_NOMAIN)))

#----------------------------------------

all: $(PATH_BIN)/$(OUT_EXE_CLIENT) $(PATH_BIN)/$(OUT_EXE_SERVER)

$(PATH_BIN)/$(OUT_EXE_CLIENT): $(PATH_OBJ)/client.o $(FILES_OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(PATH_BIN)/$(OUT_EXE_SERVER): $(PATH_OBJ)/server.o $(FILES_OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@


DEPFLAGS    = -MT $@ -MMD -MP -MF $(PATH_DEP)/$*.dTMP
POSTCOMPILE = @$(MOVE) $(PATH_DEP)/$*.dTMP $(PATH_DEP)/$*.d > $(NULL_DEVICE) && touch $@

$(PATH_OBJ)/%.o: %.c
$(PATH_OBJ)/%.o: %.c $(PATH_DEP)/%.d | directories
	$(CC) $(CPPFLAGS) -c $(DEPFLAGS) $< -o $@
	$(POSTCOMPILE)

.PRECIOUS: $(FILES_DEP)
$(FILES_DEP): ;
-include $(FILES_DEP)

#----------------------------------------

directories: $(PATH_BUILD) $(PATH_BIN) $(PATH_OBJ) $(PATH_DEP) nested-folders
nested-folders: $(addprefix $(PATH_OBJ)/, $(FOLDERS)) $(addprefix $(PATH_DEP)/, $(FOLDERS))

$(PATH_BUILD): ; $(MKDIR) $@
$(PATH_BIN): ; $(MKDIR) $@
$(PATH_OBJ): ; $(MKDIR) $@
$(PATH_DEP): ; $(MKDIR) $@

$(addprefix $(PATH_OBJ)/, $(FOLDERS)): ; @$(MKDIR) $@
$(addprefix $(PATH_DEP)/, $(FOLDERS)): ; @$(MKDIR) $@

#----------------------------------------

.PHONY: all directories nested-folders \
		clean clean-obj clean-dep clean-exe delete-build \
		run-byr run-slr help

clean: clean-obj clean-dep clean-exe
clean-obj: ; $(RMDIR) $(PATH_OBJ)/*
clean-dep: ; $(RMDIR) $(PATH_DEP)/*
clean-exe: ; $(RMDIR) $(PATH_BIN)/*
delete-build: ; $(RMDIR) $(PATH_BUILD)

ARGS ?=
run-client: ; @cd $(PATH_BIN) && ./$(OUT_EXE_CLIENT) $(ARGS)
run-server: ; @cd $(PATH_BIN) && ./$(OUT_EXE_SERVER) $(ARGS)

help:
	@echo Targets: all clean clean-obj clean-dep clean-exe delete-build run-client run-server
	@echo '(make run ARGS="arg1 arg2...")'
