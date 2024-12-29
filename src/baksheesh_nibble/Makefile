ROOT_DIR = ../..
INCLUDE_PATH = $(ROOT_DIR)/include
BUILD_DIR := $(ROOT_DIR)/build/baksheesh_nibble
CFLAGS := -I$(INCLUDE_PATH) -Wall -Werror

SRC = baksheesh.c
TEST_SRC = test.c
OBJ = $(BUILD_DIR)/baksheesh.o
TEST_BIN = $(BUILD_DIR)/test

$(shell mkdir -p $(BUILD_DIR))

.PHONY: all clean

all: $(OBJ) $(TEST_BIN)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_BIN): $(TEST_SRC) $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJ) $(TEST_BIN)