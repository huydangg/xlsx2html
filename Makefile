# The name of the source files
SOURCES =  src/read_main.c src/read_styles.c src/read_worksheet.c src/read_workbook.c

# The name of the executable
EXE = result

# Flags for compilation (adding warnings are always good)
CFLAGS = -Wall -Iinclude

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
all: $(EXE)

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
# Finally we need to tell "make" what source and header file each object file depends on
src/read_main.o: src/read_main.c include/read_main.h 
src/read_styles.o: src/read_styles.c include/read_styles.h
src/read_worksheet.o: src/read_worksheet.c include/read_worksheet.h
src/read_workbook.o: src/read_workbook.c include/read_workbook.h
