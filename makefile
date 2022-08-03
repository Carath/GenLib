##########################################################
# Settings:

# Executable name:
EXE_NAME = genlibtest

# Source and object files locations:
SRC_DIR = src
OBJ_DIR = obj

##########################################################
# Libraries:

##########################################################
# Compiler options:

# For better performance:
PROCESSOR_ARCH = -march=native

# Multithreading API:
# OPENMP = -fopenmp

# N.B: gcc for C, g++ for C++, alternative: clang.
CC := gcc
CPPFLAGS :=
CFLAGS := -std=c99 -Wall -O2 $(PROCESSOR_ARCH) $(OPENMP)
LDFLAGS :=
LDLIBS := $(OPENMP) -lm

##########################################################
# Collecting files:

# Creates the OBJ_DIR directory, if necessary:
$(shell mkdir -p $(OBJ_DIR))

# Executable, sources, objects files and dependencies:
EXE := $(EXE_NAME).exe
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEP := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.d)

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
	$(CC) -MP -MMD $(CPPFLAGS) $(CFLAGS) -c $< -o $@

-include $(DEP)

# Cleaning with 'make clean' the object files:
clean:
	rm -fv $(EXE) $(OBJ_DIR)/*.o $(OBJ_DIR)/*.d
