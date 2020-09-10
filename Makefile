CC=g++
CFLAGS=-I -Wall -std=c++11 -g
DEPS = cli_handler.h child_handler.h
OBJ = main.o cli_handler.o child_handler.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

proc_fan: $(OBJ) 
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o
