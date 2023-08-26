#bash
target:math

math: area.o test.o main.o
	g++ -o math area.o test.o main.o
area.o:
	g++ -c area.cpp
test.o:
	g++ -c test.cpp
main.o:
	g++ -c main.cpp

clean:
	rm *.o
	rm math
