# The name of the source files
SOURCES =  src/main.c src/read_styles.c src/read_worksheet.c src/read_workbook.c src/read_sharedstrings.c src/read_relationships.c src/read_drawings.c

# The name of the executable
EXE = result

MAKE_FILE_DIR = $(shell pwd)

FILES = $(shell $(MAKE_FILE_DIR)/bin/build_js.sh $(MAKE_FILE_DIR))
# Flags for compilation (adding warnings are always good)
CFLAGS = -g -Wall -Iinclude

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
	echo $(FILES)

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
src/main.o: src/main.c include/main.h 
src/read_styles.o: src/read_styles.c include/read_styles.h 
src/read_worksheet.o: src/read_worksheet.c include/read_worksheet.h
src/read_workbook.o: src/read_workbook.c include/read_workbook.h
src/read_sharedstrings.o: src/read_sharedstrings.c include/read_sharedstrings.h
src/read_relationships.o: src/read_relationships.c include/read_relationships.h
src/read_drawings.o: src/read_drawings.c include/read_drawings.h
include/private.o: include/private.h
include/const.o: include/const.h
