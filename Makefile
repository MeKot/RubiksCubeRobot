CC = gcc
CFLAGS = -Wall -pg -g -Werror -pedantic -lrt -lm -L/usr/local/lib -lwiringPi

.SUFFIXES: .c .o
.PHONY: all clean

all: $(BUILD)

solver: solver.o

clean: 
	rm -f $(wildcard *.o)
	rm -f solver
