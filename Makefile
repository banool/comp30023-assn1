# Makefile
# Daniel Porteous
# 2016

OBJ     = simulation.o process.o
CC      = gcc
EXE     = simulation
CFLAGS  = -Wall
# -O2 -m32

simulation:   $(OBJ) Makefile
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ)

clean:
	rm -f $(OBJ) $(EXE)

usage: $(EXE)
	./$(EXE)

simulation.o: process.h Makefile