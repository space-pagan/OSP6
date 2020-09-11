CC=g++
CFLAGS=-I -Wall -std=c++11 -g
DEPS = cli_handler.h child_handler.h error_handler.h
OBJ = main.o cli_handler.o child_handler.o error_handler.o

project1: proc_fan testsim
	make clean

proc_fan: $(OBJ) 
	$(CC) -o $@ $^ $(CFLAGS)

testsim: testsim.o
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o
