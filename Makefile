ask1.o: ask1.c
	gcc -c ask1.c

ask1: ask1.o
	gcc -o ask1 ask1.o

all: ask1

clean:
	rm ask1
	rm ask1.o