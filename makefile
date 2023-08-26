#bash
target:math

math: test.o main.o
	g++ -o math test.o main.o
test.o:
	g++ -c test.cpp
main.o:
	g++ -c main.cpp

clean:
	rm *.o
	rm math
