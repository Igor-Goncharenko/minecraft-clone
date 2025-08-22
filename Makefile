LIB_DIR = libs
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

SHADERS_DIR = shaders

PROJECT_NAME = minecraft-clone
VERSION = 0.0.1

CGLM_DIR = $(LIB_DIR)/cglm
CGLM_BUILD_DIR = $(CGLM_DIR)/build
CGLM_LIB = $(CGLM_BUILD_DIR)/libcglm.a

GLFW_DIR = $(LIB_DIR)/glfw
GLFW_BUILD_DIR = $(GLFW_DIR)/build
GLFW_LIB = $(GLFW_BUILD_DIR)/src/libglfw3.a

LIBS = $(CGLM_LIB) $(GLFW_LIB)

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror
CFLAGS += -Iinclude -I$(LIB_DIR)/glad/include -I$(GLFW_DIR)/include -I$(CGLM_DIR)/include
LDFLAGS = -lm $(LIBS)

SRC = $(wildcard $(SRC_DIR)/*.c)
SRC += $(LIB_DIR)/glad/src/glad.c

OBJ = $(SRC:%.c=$(OBJ_DIR)/%.o)

ifeq ($(OS), Windows_NT)
    EXEC_EXT = exe

    PROJECT_ROOT := $(shell cygpath -w -m "$(shell pwd)" | sed 's/\\/\\\\/g')

    MKDIR = if not exist $(subst /,\,$(1)) mkdir $(subst /,\,$(1))
    RMDIR = if exist $(subst /,\,$(1)) rmdir /s /q $(subst /,\,$(1))
    CP = xcopy $(1) $(2) /y /e /i
else
    EXEC_EXT = out

    PROJECT_ROOT = $(shell pwd)

    MKDIR = mkdir -p $(1)
    RMDIR = rm -rf $(1)
    CP = cp -r $(1) $(2)
endif

DEBUG_EXEC = $(BUILD_DIR)/$(PROJECT_NAME)-$(VERSION)-debug.$(EXEC_EXT)
RELEASE_EXEC = $(BUILD_DIR)/release-$(VERSION)/$(PROJECT_NAME)-$(VERSION).$(EXEC_EXT)

.PHONY: all release debug clean clean_obj

all: debug

run: release
	@echo "Starting release version..."
ifeq ($(OS), Windows_NT)
	$(RELEASE_EXEC)
else
	./$(RELEASE_EXEC)
endif

release: CFLAGS += -O3
release: clean_obj $(RELEASE_EXEC)

debug: CFLAGS += -O0 -g -DMINECRAFT_DEBUG -DPROJECT_ROOT=\"$(PROJECT_ROOT)\"
debug: clean_obj $(DEBUG_EXEC)

$(CGLM_LIB):
	@$(call MKDIR, $(@D))
	@cd $(CGLM_DIR)/build && cmake .. -DCGLM_STATIC=ON -DCGLM_SHARED=OFF
	@$(MAKE) -C $(CGLM_DIR)/build

$(GLFW_LIB):
	@$(call MKDIR, $(@D))
	@cd $(GLFW_BUILD_DIR) && cmake .. -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
	@$(MAKE) -C $(GLFW_BUILD_DIR)

$(DEBUG_EXEC): $(LIBS) $(OBJ)
	@$(call MKDIR, $(@D))
	$(CC) -o $@ $^ $(LDFLAGS)

$(RELEASE_EXEC): $(LIBS) $(OBJ)
	@$(call MKDIR, $(@D))
	@$(call CP, $(SHADERS_DIR), $(@D))
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c
	@$(call MKDIR, $(@D))
	$(CC) -o $@ -c $< $(CFLAGS)

clean_obj:
	@echo "Cleaning obj files"
	@$(call RMDIR, $(OBJ_DIR))

clean:
	@echo "Cleaning build..."
	@$(call RMDIR, $(BUILD_DIR))
