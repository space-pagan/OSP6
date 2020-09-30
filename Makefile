CC=g++
CFLAGS=-I -Wall -std=c++11 -g
DEPS = cli_handler.h child_handler.h error_handler.h shm_handler.h
EXECUTABLES = master palin 

project2: $(EXECUTABLES)

master: main.o cli_handler.o child_handler.o error_handler.o shm_handler.o
	$(CC) -o $@ $^ $(CFLAGS)

palin: palin.o shm_handler.o error_handler.o
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o $(EXECUTABLES)
