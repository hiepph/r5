###
# This Makfile is used to generate executable `center` file
# for processing video and output (x,y) of lane's center
#

# Yup, of course our precious executable file
MAIN := center

# Compile with g++, full warning and optimization level 2
CC = g++
CFLAGS = -Wall -g -O2

# Bunch of opencv library
LIBS = `pkg-config --cflags --libs opencv`

# Default warning with syntax notification
all:
	$(error SYNTAX_ERROR: $$ make build TARGET=<target>)

.PHONY: clean

# With different background target, there would be different processing
build:
	$(CC) $(CFLAGS) $(LIBS) -o $(MAIN) src/${TARGET}.cpp

# Clean all object linked, backup stuff,
# and our executable file, for a fresh start
clean:
	$(RM) *.o *~ $(MAIN)
