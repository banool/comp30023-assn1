# Makefile
# Daniel Porteous
# 2016

OBJ     = simulation.o memory.o process.o 
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

simulation.o: memory.h process.h Makefile
memory.o: process.h