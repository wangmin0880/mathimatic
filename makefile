#bash
target:app lib

app: math
math: area.o test.o main.o
	g++ -o math area.o test.o main.o
area.o:
	g++ -c src/area.cpp -I./ -o area.o
test.o:
	g++ -c test/test.cpp -I./ -I./src/
main.o:
	g++ -c test/main.cpp -I./ -I./src/

lib: so ar
so:
	g++ -fPIC -shared -o libmathimatic.so src/area.cpp -I./
ar: area.o
	ar rc libmathimatic.a area.o

clean:
	rm *.o math
	rm *.a *.so
