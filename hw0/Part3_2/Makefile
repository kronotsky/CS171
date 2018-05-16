###############################################################################
# CS/CNS 171 Fall 2015
#
# This is a template Makefile for the assignment. Edit it however you find
# convenient.
###############################################################################
CC = g++
FLAGS = -g -std=c++11

# The following line is a relative directory reference that assumes the Eigen
# folder--which your program will depend on--is located one directory above the
# directory that contains this Makefile.
# Placed Eigen in /usr/local/include, so this line is unnecessary:
# INCLUDE = -I../

# No *.h files:
SOURCES = *.cpp *.hh

EXENAME = myprog

all: $(SOURCES)
	$(CC) $(FLAGS) -o $(EXENAME) $(SOURCES)

# Originally $(CC) $(FLAGS) -o $(EXENAME) $(INCLUDE) $(SOURCES)

clean:
	rm -f *.o $(EXENAME)

.PHONY: all clean

