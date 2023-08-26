#bash
target:lib app

app: math
math: test.o main.o
	g++ -o math test.o main.o -L. -lmathimatic
test.o:
	g++ -c test/test.cpp -I./ -I./include
main.o:
	g++ -c test/main.cpp -I./ -I./include

lib: dot_so dot_a
dot_so:
	g++ -fPIC -shared -o libmathimatic.so src/area.cpp -I./ -I./include
dot_a: area.o
	ar rc libmathimatic.a area.o
area.o:
	g++ -c src/area.cpp -o area.o -I./ -I./include

install:
#	sudo mkdir /usr/local/lib/mathimatic
	sudo cp libmathimatic.so /usr/local/lib/mathimatic/
	sudo cp libmathimatic.conf /etc/ld.so.conf.d/
	sudo ldconfig
clean:
	rm *.o math
	rm *.a *.so
