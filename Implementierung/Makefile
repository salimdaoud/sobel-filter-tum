# ---------------------------------------
# CONFIGURATION BEGIN
# ---------------------------------------

# entry point for the program and target name
MAIN := main.c

# assignment task file TODO: make this find the files automatically
IMPLEMENTIERUNG :=	$(wildcard conversion/*.c) \
					$(wildcard io/*.c) \
					$(wildcard util/*.c) \
					$(wildcard test/*.c)

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
SOURCES := $(MAIN) $(IMPLEMENTIERUNG)
TARGET := $(basename $(MAIN))

# Default to release build for both app and library
all: debug

# Debug build
debug: CFLAGS += -g -O0
debug: $(TARGET)

# Release build
release: CFLAGS += -O2 
release: $(TARGET)

# recipe for building the program
$(TARGET): $(SOURCES)
	$(CC)  -msse4.1 $(CFLAGS) -o $@ $(SOURCES)  -lm

# clean up
clean:
	rm -f $(TARGET)

.PHONY: all debug release clean

# TODO: automate performance tests