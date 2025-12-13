# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11

TARGET = bioc

# C source files
C_SOURCES = bioc.c

# Object files
C_OBJECTS = $(C_SOURCES:.c=.o)

# Default target
all: $(TARGET)

# Link everything
$(TARGET): $(C_OBJECTS)
	$(CC) $(CFLAGS) $(C_OBJECTS) -o $(TARGET)

# Compile C source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
.PHONY: clean
clean:
	rm -f $(TARGET) $(C_OBJECTS)
