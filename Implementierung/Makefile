# ---------------------------------------
# CONFIGURATION BEGIN
# ---------------------------------------

# entry point for the program and target name
MAIN := main.c

# assignment task file TODO: make this find the files automatically
ASSIGNMENT := 	conversion/sobel.c \
				conversion/grayscale.c \
				io/arg_parser.c \
				io/readwrite.c \
				util/lookup_tables.c \
				util/square_root.c

# Additional flags for the compiler
CFLAGS := -Wall -Wextra -std=c17

# ---------------------------------------
# CONFIGURATION END
# ---------------------------------------

# Determine if clang or gcc is available
CC := $(shell command -v gcc || command -v clang)
ifeq ($(strip $(CC)),)
    $(error Neither clang nor gcc is available. Exiting.)
endif

# all source files needed for the program
SOURCES := $(MAIN) $(ASSIGNMENT)
TARGET := $(basename $(MAIN))

# Default to release build for both app and library
all: debug

# Debug build
debug: CFLAGS += -g -o0
debug: $(TARGET)

# Release build
release: CFLAGS += -O2
release: $(TARGET)

# recipe for building the program
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES)  -lm

# clean up
clean:
	rm -f $(TARGET)

.PHONY: all debug release clean

# TODO: automate performance tests