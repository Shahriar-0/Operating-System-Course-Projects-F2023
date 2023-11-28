include common_vars.mk

EXE_COMPANY   := company.out
EXE_FINANCIAL := financial.out
EXE_BUILDING  := building.out
EXE_RESOURCE     := resource.out

VPATH = $(PATH_SRC)

FILES   = $(patsubst src/%, %, $(shell find $(PATH_SRC) -name "*.cpp" -type f))
FOLDERS = $(patsubst src/%, %, $(shell find $(PATH_SRC) -mindepth 1 -type d))

FILES_NOMAIN = $(filter-out company_processor.cpp financial_processor.cpp building_processor.cpp resource_processor.cpp, $(FILES))

FILES_DEP = $(patsubst %, $(PATH_DEP)/%.d, $(basename $(FILES)))
FILES_OBJ = $(patsubst %, $(PATH_OBJ)/%.o, $(basename $(FILES_NOMAIN)))

#----------------------------------------

all: $(PATH_BIN)/$(EXE_COMPANY) $(PATH_BIN)/$(EXE_FINANCIAL) $(PATH_BIN)/$(EXE_BUILDING) $(PATH_BIN)/$(EXE_RESOURCE)

$(PATH_BIN)/$(EXE_COMPANY): $(PATH_OBJ)/company_processor.o $(FILES_OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(PATH_BIN)/$(EXE_FINANCIAL): $(PATH_OBJ)/financial_processor.o $(FILES_OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(PATH_BIN)/$(EXE_BUILDING): $(PATH_OBJ)/building_processor.o $(FILES_OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(PATH_BIN)/$(EXE_RESOURCE): $(PATH_OBJ)/resource_processor.o $(FILES_OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@


DEPFLAGS    = -MT $@ -MMD -MP -MF $(PATH_DEP)/$*.dTMP
POSTCOMPILE = @$(MOVE) $(PATH_DEP)/$*.dTMP $(PATH_DEP)/$*.d > $(NULL_DEVICE) && touch $@

$(PATH_OBJ)/%.o: %.cpp
$(PATH_OBJ)/%.o: %.cpp $(PATH_DEP)/%.d | directories
	$(CC) $(CPPFLAGS) -c $(DEPFLAGS) $< -o $@
	$(POSTCOMPILE)

.PRECIOUS: $(FILES_DEP)
$(FILES_DEP): ;
-include $(FILES_DEP)

#----------------------------------------

directories: $(PATH_BUILD) $(PATH_BIN) $(PATH_OBJ) $(PATH_DEP) nested-folders
nested-folders: $(addprefix $(PATH_OBJ)/, $(FOLDERS)) $(addprefix $(PATH_DEP)/, $(FOLDERS))

$(PATH_BUILD): ; $(MKDIR) $@
$(PATH_BIN)  : ; $(MKDIR) $@
$(PATH_OBJ)  : ; $(MKDIR) $@
$(PATH_DEP)  : ; $(MKDIR) $@

$(addprefix $(PATH_OBJ)/, $(FOLDERS)): ; @$(MKDIR) $@
$(addprefix $(PATH_DEP)/, $(FOLDERS)): ; @$(MKDIR) $@

#----------------------------------------

.PHONY: all directories nested-folders \
		clean clean-obj clean-dep clean-exe delete-build \
		run help

clean: clean-obj clean-dep clean-exe clean-fifos clean-logs
clean-obj   : ; $(RMDIR) $(PATH_OBJ)/*
clean-dep   : ; $(RMDIR) $(PATH_DEP)/* 
clean-fifos : ; $(RMDIR) $(PATH_BUILD)/fifos/* 
clean-logs  : ; $(RMDIR) $(PATH_BUILD)/logs/*
clean-exe   : ; $(RMDIR) $(PATH_BIN)/$(EXE_COMPANY) $(PATH_BIN)/$(EXE_FINANCIAL) $(PATH_BIN)/$(EXE_BUILDING) $(PATH_BIN)/$(EXE_RESOURCE)

ARGS ?=
run: ; @cd $(PATH_BIN) && ./$(EXE_COMPANY) "../../buildings"

help:
	@echo Targets: all clean clean-obj clean-dep clean-exe delete-build run
	@echo '(make run ARGS="arg1 arg2...")'