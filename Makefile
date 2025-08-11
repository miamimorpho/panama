
TARGET = solution
LIBS = -lm
CC = cc
CFLAGS = -g -Wall -Wextra -Wpedantic

# Directory structure
SRCDIR = src
OBJDIR = obj

.PHONY: default all clean

default: $(TARGET)
all: default

# Find all source files in src directory
SOURCES = $(wildcard $(SRCDIR)/*.c)
# Convert source paths to object paths in obj directory (portable method)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
# Find all headers in src directory
HEADERS = $(wildcard $(SRCDIR)/*.h)

# Create obj directory if it doesn't exist
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Rule to compile .c files to .o files in obj directory
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link all object files to create the target executable
.PRECIOUS: $(TARGET) $(OBJECTS)
$(TARGET): $(OBJECTS)
	$(CC) -g $(OBJECTS) $(LIBS) -o $@

# Clean up generated files
clean:
	-rm -rf $(OBJDIR)
	-rm -f $(TARGET)
