CC=g++
CFLAGS=-I -Wall -std=c++11 -g
DEPS = cli_handler.h child_handler.h error_handler.h
OBJ = main.o cli_handler.o child_handler.o error_handler.o
EXECUTABLES = master palin 

project2: $(EXECUTABLES)

master: $(OBJ) 
	$(CC) -o $@ $^ $(CFLAGS)

palin: palin.o
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o $(EXECUTABLES)
