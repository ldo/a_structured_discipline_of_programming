# This Makefile is now used for testing only.

PYTHONVER=python3.8
CFLAGS=-g -I/usr/include/${PYTHONVER} -fPIC -Wall -Wno-parentheses

discipline.so : discipline.o
	$(CC) $^ -L${PYTHONVER}/config -l${PYTHONVER} -shared -o $@

discipline.o : discipline.c

clean :
	rm -rf discipline.so discipline.o build/ __pycache__/

.PHONY : clean
