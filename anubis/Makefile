# Feel free to extend this Makefile as you see fit. Just ensure that
# running `make` will compile your code to a binary named `anubis`.

CC = gcc
CFLAGS = -Wall -g -O0
OFLAGES = -Wall -c

SRC_DIR ?= ./src
BUILD_DIR ?= .
SRCS := $(shell find $(SRC_DIR) -name *.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
HEADERS := $(shell find $(SRC_DIR) -name *.h)


# c source
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

anubis: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

clean:
	rm $(BUILD_DIR)/*.o
	$(RM) anubis 

.PHONY: clean
