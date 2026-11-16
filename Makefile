CC      = gcc
CFLAGS  = -Wall -g -pthread -MMD -MP
LDFLAGS = -pthread

SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

TARGET = $(BIN_DIR)/sysmond

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

run: $(TARGET)
	./$(TARGET) $(INTERVAL)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)/*

-include $(DEPS)

.PHONY: all run clean
