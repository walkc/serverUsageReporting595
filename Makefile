CC = clang
ARGS = -Wall -g

all: clean server

server: project.c
	$(CC) -o server $(ARGS) -pthread server.c project.c

clean: 
	rm -rf server