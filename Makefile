CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic

all = build

build:  
	$(CC) setup.c spawn_shell.c myinit.c -o myinit

install:
	mv myinit /bin/myinit

.PHONY: clean
clean:
	rm -f myinit
