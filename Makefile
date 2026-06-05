# Use GCC as the C compiler.
CC ?= gcc

# Add warning flags, use C11, and optimize the output.
CFLAGS ?= -Wall -Wextra -Werror -std=c11 -O2

# Add preprocessor flags (here we add include directory).
CPPFLAGS ?= -Iinclude

# Linker flags (empty now, add things like -Wl,... if needed).
LDFLAGS ?=

# Libraries to link (empty now, add things like -lm or -lpthread).
LDLIBS ?=

# Archiver for static libraries.
AR ?= ar

# Final static library file name.
TARGET ?= $(BUILD_DIR)/libfile_reader.a

# Folder that contains .c files.
SRC_DIR ?= src

# Folder where compiled .o files are stored.
BUILD_DIR ?= build

# Test folder and test binary name.
TEST_DIR ?= test
TEST_TARGET ?= $(BUILD_DIR)/test_runner

# Find all .c files inside SRC_DIR.
SRCS := $(wildcard $(SRC_DIR)/*.c)

# Find all .c files inside TEST_DIR.
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)

# Convert src/file.c to build/file.o for every source file.
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Convert test/file.c to build/file.test.o for every test source file.
TEST_OBJS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%.test.o,$(TEST_SRCS))

# Auto-generated dependency files for sources and tests.
DEPS := $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)

# Default target: running `make` builds TARGET.
all: $(TARGET)

# TARGET depends on all object files.
$(TARGET): $(OBJS)
# Archive all object files into the static library.
	$(AR) rcs $@ $^

# Pattern rule: describes how to build one .o from one .c.
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
# Compile one source file ($<) into one object file ($@).
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

# Pattern rule for test object files.
$(BUILD_DIR)/%.test.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
# Compile one test source file into one test object file.
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

# Rule to ensure build directory exists before compiling objects.
$(BUILD_DIR):
# Create directory and do nothing if it already exists.
	mkdir -p $(BUILD_DIR)

# Convenience target to run the program after it is built.
run: $(TARGET)
# No executable is built by default in this library-only project.
	@echo "No runnable binary target is configured. Build is a static library: $(TARGET)"

# Build a test binary if test sources exist.
$(TEST_TARGET): $(OBJS) $(TEST_OBJS)
# Link production and test objects into test runner.
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Build and run tests when test sources exist.
test: $(TEST_TARGET)
# Execute the test runner binary.
	./$(TEST_TARGET)

# Convenience target to remove generated files.
clean:
# Delete build directory and executable.
	rm -rf $(BUILD_DIR) $(TARGET)

# Print key Make variables (useful when learning/debugging).
print-vars:
	@echo "CC=$(CC)"
	@echo "CFLAGS=$(CFLAGS)"
	@echo "CPPFLAGS=$(CPPFLAGS)"
	@echo "LDFLAGS=$(LDFLAGS)"
	@echo "LDLIBS=$(LDLIBS)"
	@echo "SRCS=$(SRCS)"
	@echo "OBJS=$(OBJS)"

# Show available targets.
help:
	@echo "Targets: all run test clean print-vars help"

# Mark these as phony so make does not confuse them with real files.
.PHONY: all run test clean print-vars help

# Include auto-generated header dependency files if they exist.
-include $(DEPS)

# Useful automatic variables inside recipes:
# $@ means current target name.
# $< means first prerequisite.
# $^ means all prerequisites.

# Example of automatic variables usage:
# $(CC) -c $< -o $@

