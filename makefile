##########################################################
# Settings:

# Executable name:
EXE_NAME = genlibtest

# Source and objects files location:
SRC_DIR = src
OBJ_DIR = obj

##########################################################
# Libraries:

##########################################################
# Compiler options:

# N.B: gcc for C, g++ for C++, alternative: clang.
CC = gcc
CPPFLAGS =
CFLAGS = -std=c99 -Wall -O2
LDFLAGS =
LDLIBS = -lm

##########################################################
# Collecting files:

# Creates the OBJ_DIR directory, if necessary:
$(shell mkdir -p $(OBJ_DIR))

EXE = $(EXE_NAME).exe

# Sources and objects files:
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

##########################################################
# Compilation rules:

# The following names are not associated with files:
.PHONY: all clean

# All executables to be created:
all: $(EXE)

# Linking the program:
$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Compiling the source files:
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Cleaning with 'make clean' the object files:
clean:
	rm -fv $(EXE) $(OBJ_DIR)/*
