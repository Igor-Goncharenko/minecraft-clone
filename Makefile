LIB_DIR = libs
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

PROJECT_NAME= minecraft-clone
VERSION = 0.0.1

DEBUG_EXEC = $(BUILD_DIR)/debug-$(VERSION)/$(PROJECT_NAME)-$(VERSION)-debug.out
RELEASE_EXEC = $(BUILD_DIR)/release-$(VERSION)/$(PROJECT_NAME)-$(VERSION).out

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror
CFLAGS += -Iinclude -I$(LIB_DIR)/glad/include
LDFLAGS = -lm -ldl -lpthread

SRC = $(wildcard $(SRC_DIR)/*.c)
SRC += $(LIB_DIR)/glad/src/glad.c

OBJ = $(SRC:.c=.o)

CGLM_DIR = $(LIB_DIR)/cglm
CGLM_BUILD_DIR = $(CGLM_DIR)/build
CGLM_LIB = $(CGLM_BUILD_DIR)/libcglm.a

GLFW_DIR = $(LIB_DIR)/glfw
GLFW_BUILD_DIR = $(GLFW_DIR)/build
GLFW_LIB = $(GLFW_BUILD_DIR)/src/libglfw3.a

LIBS = $(CGLM_LIB) $(GLFW_LIB)

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
release: $(RELEASE_EXEC)

debug: CFLAGS += -O0 -g -DMC_DEBUG
debug: $(DEBUG_EXEC)

$(CGLM_LIB):
	@mkdir -p $(CGLM_DIR)/build
	@cd $(CGLM_DIR)/build && cmake .. -DCGLM_STATIC=ON -DCGLM_SHARED=OFF
	@$(MAKE) -C $(CGLM_DIR)/build

$(GLFW_LIB):
	@mkdir -p $(GLFW_BUILD_DIR)
	@cd $(GLFW_BUILD_DIR) && cmake .. -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF
	@$(MAKE) -C $(GLFW_BUILD_DIR)

$(DEBUG_EXEC): $(LIBS) $(OBJ)
	@mkdir -p $(@D)
	$(CC) -o $@ $^ $(LDFLAGS)

$(RELEASE_EXEC): $(LIBS) $(OBJ)
	@mkdir -p $(@D)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/obj/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	@rm -rf $(BUILD_DIR)
