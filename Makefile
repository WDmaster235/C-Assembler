# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g
INCLUDES = -Iinclude
OBJDIR = build
SRC = src/main.c src/preassembler.c src/parser.c src/encoder.c src/io.c src/macro.c src/commands.c src/label.c
OBJ = $(OBJDIR)/main.o $(OBJDIR)/preassembler.o $(OBJDIR)/parser.o $(OBJDIR)/encoder.o \
      $(OBJDIR)/io.o $(OBJDIR)/macro.o $(OBJDIR)/commands.o $(OBJDIR)/label.o

# Targets
EXEC = assembler
TEST_EXEC = assembler_test

# Default target
all: $(EXEC)

# Production build
$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ)

# Testing build
test: CFLAGS += -DTEST_MODE
test: $(OBJ)
	$(CC) $(CFLAGS) -o $(TEST_EXEC) $(OBJ)

# Object file compilation with modified CFLAGS
$(OBJDIR)/%.o: src/%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(OBJDIR) $(EXEC) $(TEST_EXEC)

# Phony targets
.PHONY: all test clean
