TARGET = rogue
LIBS = -lm
CC = cc
CFLAGS = -g -Wall -Wextra -Wpedantic -Wunused-macros -Wunused-function -fdiagnostics-color=always

# Directory structure
SRCDIR = src
EXTERNDIR = extern
OBJDIR = obj

.PHONY: default all clean extern

default: $(TARGET)
all: default

# Find all source files in src and extern directories
SOURCES = $(wildcard $(SRCDIR)/*.c)
EXTERN_SOURCES = $(wildcard $(EXTERNDIR)/*.c)

# Convert source paths to object paths in obj directory
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
EXTERN_OBJECTS = $(patsubst $(EXTERNDIR)/%.c,$(OBJDIR)/%.o,$(EXTERN_SOURCES))

# Combine all objects
ALL_OBJECTS = $(OBJECTS) $(EXTERN_OBJECTS)

# Find all headers in src and extern directories
HEADERS = $(wildcard $(SRCDIR)/*.h)
EXTERN_HEADERS = $(wildcard $(EXTERNDIR)/*.h)

# Create obj directory if it doesn't exist
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Rule to compile .c files from src directory to .o files in obj directory
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to compile .c files from extern directory to .o files in obj directory
$(OBJDIR)/%.o: $(EXTERNDIR)/%.c $(EXTERN_HEADERS) $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Separate target to compile only extern files
extern: $(EXTERN_OBJECTS)

# Link all object files to create the target executable
.PRECIOUS: $(TARGET) $(ALL_OBJECTS)
$(TARGET): $(ALL_OBJECTS)
	$(CC) -g $(ALL_OBJECTS) $(LIBS) -o $@

# Clean up generated files
clean:
	-rm -rf $(OBJDIR)
	-rm -f $(TARGET)
