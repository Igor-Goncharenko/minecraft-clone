LIB_DIR = libs
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

PROJECT_ROOT := $(shell pwd)

PROJECT_NAME= minecraft-clone
VERSION = 0.0.1

CGLM_DIR = $(LIB_DIR)/cglm
CGLM_BUILD_DIR = $(CGLM_DIR)/build
CGLM_LIB = $(CGLM_BUILD_DIR)/libcglm.a

GLFW_DIR = $(LIB_DIR)/glfw
GLFW_BUILD_DIR = $(GLFW_DIR)/build
GLFW_LIB = $(GLFW_BUILD_DIR)/src/libglfw3.a

SQLITE_DIR = $(LIB_DIR)/sqlite
SQLITE_LIB = $(SQLITE_DIR)/sqlite3.c
SQLITE_OBJ = $(OBJ_DIR)/sqlite3.o
SQLITE_CFLAGS = -std=c99 -O1 -I$(SQLITE_DIR) -Wno-stringop-overread -Wno-implicit-fallthrough

LIBS = $(CGLM_LIB) $(GLFW_LIB) $(SQLITE_OBJ)

DEBUG_EXEC = $(BUILD_DIR)/debug-$(VERSION)/$(PROJECT_NAME)-$(VERSION)-debug.out
RELEASE_EXEC = $(BUILD_DIR)/release-$(VERSION)/$(PROJECT_NAME)-$(VERSION).out

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror
CFLAGS += -Iinclude -I$(LIB_DIR)/glad/include -I$(GLFW_DIR)/include -I$(CGLM_DIR)/include -I$(SQLITE_DIR)
CFLAGS += -DPROJECT_ROOT=\"$(PROJECT_ROOT)\"
LDFLAGS = -lm -ldl -lpthread $(LIBS)

SRC = $(wildcard $(SRC_DIR)/*.c)
SRC += $(LIB_DIR)/glad/src/glad.c

OBJ = $(SRC:%.c=$(OBJ_DIR)/%.o)

.PHONY: all release debug valgrind gdb clean

all: release

gdb: debug
	@echo "Starting GDB..."
	@gdb -tui ./$(DEBUG_EXEC)

valgrind: debug
	@echo "Starting valgrind..."
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(DEBUG_EXEC)

run: release
	@echo "Starting release version..."
	./$(RELEASE_EXEC)

release: CFLAGS += -O3
release: clean_obj $(RELEASE_EXEC)

debug: CFLAGS += -O0 -g -DMC_DEBUG
debug: clean_obj $(DEBUG_EXEC)

$(CGLM_LIB):
	@mkdir -p $(CGLM_DIR)/build
	@cd $(CGLM_DIR)/build && cmake .. -DCGLM_STATIC=ON -DCGLM_SHARED=OFF
	@$(MAKE) -C $(CGLM_DIR)/build

$(GLFW_LIB):
	@mkdir -p $(GLFW_BUILD_DIR)
	@cd $(GLFW_BUILD_DIR) && cmake .. -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
	@$(MAKE) -C $(GLFW_BUILD_DIR)

$(SQLITE_LIB):
	@cd $(SQLITE_DIR) && ./configure
	@cd $(SQLITE_DIR) && $(MAKE) sqlite3.c

$(SQLITE_OBJ): $(SQLITE_LIB)
	@mkdir -p $(@D)
	$(CC) $(SQLITE_CFLAGS) -c $< -o $@

$(DEBUG_EXEC): $(LIBS) $(OBJ)
	@mkdir -p $(@D)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

$(RELEASE_EXEC): $(LIBS) $(OBJ)
	@mkdir -p $(@D)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -o $@ -c $< $(CFLAGS)

clean_obj:
	@echo "Cleaning obj files"
	@rm -rf $(OBJ_DIR)

clean:
	@rm -rf $(BUILD_DIR)
