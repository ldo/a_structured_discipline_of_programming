# Build discipline extension module.

# may need to fix PYTHONVER as needed for newer/older Python
PYTHONVER=python3.8
CFLAGS=-g -I/usr/include/${PYTHONVER} -fPIC -Wall -Wno-parentheses

discipline.so : discipline.o
	$(CC) $^ -L${PYTHONVER}/config -l${PYTHONVER} -shared -o $@

discipline.o : discipline.c

clean :
	rm -f discipline.so discipline.o

.PHONY : clean
