#bash
target:math

math: area.o test.o main.o
	g++ -o math area.o test.o main.o
area.o:
	g++ -c src/area.cpp -I./ -I./test/
test.o:
	g++ -c test/test.cpp -I./ -I./src/
main.o:
	g++ -c test/main.cpp -I./ -I./src/

clean:
	rm *.o
	rm math
