# ---------------------------------------
# CONFIGURATION BEGIN
# ---------------------------------------

# entry point for the program and target name
MAIN := main.c

# assignment task file
IMPLEMENTIERUNG :=	$(wildcard conversion/*.c) \
					$(wildcard io/*.c) \
					$(wildcard util/*.c) \
					$(wildcard test/*.c)

# Additional flags for the compiler
CFLAGS := -Wall -Wextra -std=c17 -msse4.1

# ---------------------------------------
# CONFIGURATION END
# ---------------------------------------

# Determine if clang or gcc is available
CC := $(shell command -v gcc || command -v clang)
ifeq ($(strip $(CC)),)
    $(error Neither clang nor gcc is available. Exiting.)
endif

# all source files needed for the program
SOURCES := $(MAIN) $(IMPLEMENTIERUNG)
TARGET := $(basename $(MAIN))

# Default to release build for both app and library
all: release

# Debug build
debug: CFLAGS += -g -O0
debug: $(TARGET)

# Release build
release: CFLAGS += -O2
release: $(TARGET)

# Performance Benchmarking
perf: CFLAGS += -O2 -pg
perf: $(TARGET)
	  @echo "Running performance benchmarks..."

# recipe for building the program
$(TARGET): $(SOURCES)
	$(CC)  $(CFLAGS) -o $@ $(SOURCES)  -lm

# clean up
clean:
	rm -f $(TARGET)

.PHONY: all debug release perf clean