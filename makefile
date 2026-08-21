CC=gcc
FLAGS=-Wall -Wextra -g
SANTS=-fsanitize=address,undefined
INCLUDE_FLAGS=`curl-config --cflags --libs`

SRC_DIR=src
BUILD_DIR=build
BIN_DIR=bin

NAME=main.uwu

SOURCES_REG=$(wildcard $(SRC_DIR)/*.c)
SOURCES_DIR=$(wildcard $(SRC_DIR)/*/)
SOURCES_IND=$(wildcard $(SRC_DIR)/*/*.c)

SOURCES=$(SOURCES_REG) $(SOURCES_IND)

BUILD_DIRS=$(SOURCES_DIR:$(SRC_DIR)/%/=$(BUILD_DIR)/%/)
OBJECTS=$(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR) $(BUILD_DIRS) $(BIN_DIR)/$(NAME)

$(BIN_DIR)/$(NAME): $(BIN_DIR) $(OBJECTS)
	$(CC) $(INCLUDE_FLAGS) $(OBJECTS) $(FLAGS) $(QUIET) $(SANTS) -o $(BIN_DIR)/$(NAME)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(INCLUDE_FLAGS) $(FLAGS) $^ -o $@ -c $(QUIET)

$(BUILD_DIRS): $(BUILD_DIR)
	mkdir -p "$@"

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)

launch:
	./$(BIN_DIR)/$(NAME)

cleanbuild:
	make clean
	make FLAGS= SANTS= 

full:
	make clean
	make
	make launch

manpage: docs/pacboy.1
	cp docs/pacboy.1 /usr/share/man/man1/pacboy.1
