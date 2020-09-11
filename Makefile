CC=g++
CFLAGS=-I -Wall -std=c++11 -g
DEPS = cli_handler.h child_handler.h error_handler.h
OBJ = main.o cli_handler.o child_handler.o error_handler.o
EXECUTABLES = proc_fan testsim testsim2

project1: $(EXECUTABLES)

proc_fan: $(OBJ) 
	$(CC) -o $@ $^ $(CFLAGS)

testsim: testsim.o
	$(CC) -o $@ $^ $(CFLAGS)

testsim2: testsim2.o
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o $(EXECUTABLES)
