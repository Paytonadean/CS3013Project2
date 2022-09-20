all: problem1 problem2

problem1: problem1.o
	gcc -o problem1 problem1.o

problem1.o: problem1.c
	gcc -c problem1.c -pthread

problem2: problem2.o
	gcc -o problem2 problem2.o

generation.o: problem2.c
	gcc -c problem2.c -pthread

clean:
	rm problem1.o problem1 problem2.o problem2