ifeq ($(OS),Windows_NT)
    HOST := win
else
    HOST := $(if $(filter Darwin,$(shell uname -s)),mac,linux)
endif

PLATFORM ?= $(HOST)

ifeq ($(origin CC),default)
    CC := gcc
endif

ifeq ($(OS),Windows_NT)
    ifeq ($(origin CMD_SHELL),undefined)
        ifeq ($(MSYSTEM),)
            CMD_SHELL := 1
        endif
    endif
endif

ifdef CMD_SHELL
    SHELL := cmd.exe
    winpath = $(subst /,\,$(patsubst %/,%,$1))
    MKDIR = if not exist "$(call winpath,$1)" mkdir "$(call winpath,$1)"
    RMDIR = if exist "$(call winpath,$1)" rmdir /s /q "$(call winpath,$1)"
    RUN = $(call winpath,$1)
else
    MKDIR = mkdir -p $1
    RMDIR = rm -rf $1
    RUN = ./$1
endif

LDLIBS := -lm
EXE :=

ifeq ($(PLATFORM),win)
    EXE := .exe
    LDFLAGS += -static
    LDLIBS += -lshlwapi
endif

OUT_FILE := ass
SRC_DIR := src
BUILD_DIR := build
OBJ_DIR := obj/$(PLATFORM)

CCFLAGS := -g -O2 -Wall -Wextra -Werror -std=gnu11 -Iinclude -MMD -MP

rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SOURCES := $(call rwildcard,$(SRC_DIR)/,*.c)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS := $(OBJECTS:.o=.d)
TARGET := $(BUILD_DIR)/$(OUT_FILE)$(EXE)

SAMPLE_PROJ_SRC := tests/sample_proj/src/main.ass

.PHONY: all run clean vars

build: $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(call MKDIR,$(dir $@))
	$(CC) $(CCFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	@$(call MKDIR,$(BUILD_DIR))
	$(CC) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

run: build
	$(call RUN,$(TARGET)) $(ARGS)

tests: build
	ARGS="tests"
	$(MAKE) run

vars:
	@echo Platform: $(PLATFORM) (host: $(HOST))
	@echo Shell: $(SHELL) (cmd mode: $(if $(CMD_SHELL),yes,no))
	@echo Sources: $(SOURCES)
	@echo Objects: $(OBJECTS)

clean:
	-$(call RMDIR,$(BUILD_DIR))
	-$(call RMDIR,obj)

-include $(DEPS)
