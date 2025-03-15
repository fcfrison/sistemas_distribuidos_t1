# Compiler and flags
# No vs code, utilizar crtl+shift+b
CC        := gcc-13
CFLAGS    := -Wall -Wextra -g  # Added warning flags for better code quality

# Directory structure
BIN      := bin
SRC      := src
INCLUDE  := include
TESTS    := tests
PP2PLINK := PP2PLink
# Libraries and executable name
EXECUTABLE := server
TEST_EXECUTABLE := tests

# Source files
SRC_FILES := $(wildcard $(SRC)/*.c) $(wildcard $(SRC)/$(PP2PLINK)/*.c)
TEST_FILES := $(TESTS)/test_chan.c

# Main target
all: $(BIN)/$(EXECUTABLE)

# Run target - cleans, rebuilds, and executes the program
run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

# Compilation rule
# Note: $^ represents all prerequisites (source files)
# Note: $@ represents the target (the executable)
$(BIN)/$(EXECUTABLE): $(SRC_FILES)
	$(CC) $(CFLAGS) -I$(INCLUDE) $^ -o $@

# Compilation rule for the test executable
$(BIN)/$(TEST_EXECUTABLE): $(filter-out $(SRC)/app.c, $(SRC_FILES)) $(TEST_FILES)
	$(CC) $(CFLAGS) -I$(INCLUDE) $^ -o $@

# Test target - builds and runs the test executable
test: $(BIN)/$(TEST_EXECUTABLE)
	./$(BIN)/$(TEST_EXECUTABLE)

# Clean rule - removes compiled files
# The - before rm makes make continue even if there's nothing to remove
clean:
	-rm $(BIN)/*