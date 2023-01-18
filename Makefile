# The name of the source files
SOURCES = lib/whereami.c src/main.c src/style.c src/sheet.c src/book.c src/sharedstring.c src/relationship.c src/drawing.c src/chart.c src/ssfc.c

# The name of the executable
EXE = xlsx2html

MAKE_FILE_DIR = $(shell pwd)

FILES_JS = $(MAKE_FILE_DIR)/bin/build_js.sh $(MAKE_FILE_DIR)
FILES_CSS = $(MAKE_FILE_DIR)/bin/build_css.sh $(MAKE_FILE_DIR)
FILES_SSF = $(MAKE_FILE_DIR)/bin/build_ssf.sh $(MAKE_FILE_DIR)

# Flags for compilation (adding warnings are always good)
CFLAGS = -g -Wall -Iinclude -Ilib -std=gnu11

# Flags for linking (none for the moment)
LDFLAGS =

# Libraries to link with (none for the moment)
LIBS = -lzip -lexpat -lm
# Use the GCC frontend program when linking
LD = gcc

# This creates a list of object files from the source files
OBJECTS = $(SOURCES:%.c=%.o)

# The first target, this will be the default target if none is specified
# This target tells "make" to make the "all" target
default: all

# Having an "all" target is customary, so one could write "make all"
# It depends on the executable program
all:	$(EXE)
	$(FILES_JS)
	$(FILES_CSS)

# This will link the executable from the object files
$(EXE): $(OBJECTS)
	$(LD) $(LDFLAGS) $(CFLAGS) $(OBJECTS) -o  $(EXE) $(LIBS)

# This is a target that will compiler all needed source files into object files
# We don't need to specify a command or any rules, "make" will handle it automatically
src/%.o: src/%.c

# Target to clean up after us
clean:
	-rm -f $(EXE)      # Remove the executable file
	-rm -f $(OBJECTS)  # Remove the object files
	-rm -rf output/*  # Remove the object files
	-rm -f $(MAKE_FILE_DIR)/templates/*.min.js # Remove the templates files
	-rm -f $(MAKE_FILE_DIR)/templates/*.min.css # Remove the templates files
# Finally we need to tell "make" what source and header file each object file depends on
lib/whereami.o: lib/whereami.c lib/whereami.h
src/main.o: src/main.c include/main.h 
src/style.o: src/style.c include/style.h 
src/sheet.o: src/sheet.c include/sheet.h
src/book.o: src/book.c include/book.h
src/sharedstring.o: src/sharedstring.c include/sharedstring.h
src/relationship.o: src/relationship.c include/relationship.h
src/drawing.o: src/drawing.c include/drawing.h
src/chart.o: src/chart.c include/chart.h
src/ssfc.o: src/ssfc.c include/ssfc.h
include/private.o: include/private.h
include/const.o: include/const.h
include/version.o: include/version.h
