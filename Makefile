# Build discipline extension module.

CFLAGS=-g $(shell python3-config --includes) -fPIC -Wall -Wno-parentheses

discipline.so : discipline.o
	$(CC) $^ $(shell python3-config --ldflags) -shared -o $@

discipline.o : discipline.c

clean :
	rm -f discipline.so discipline.o

.PHONY : clean
