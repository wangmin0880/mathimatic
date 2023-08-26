#/bin/bash
target:lib app

app: math
math: test.o main.o area.o math.o
#	g++ -o math test.o main.o -L. -lmathimatic
	g++ -o math test.o main.o area.o math.o
test.o:
	g++ -c test/test.cpp -I./ -I./include
main.o:
	g++ -c test/main.cpp -I./ -I./include

lib: dot_so dot_a
dot_so:
	g++ src/area.cpp src/math.cpp -fPIC -shared -o libmathimatic.so -I./ -I./include
dot_a: area.o math.o
	ar rc libmathimatic.a area.o
area.o:
	g++ -c src/area.cpp -o area.o -I./ -I./include
math.o:
	g++ -c src/math.cpp -o math.o -I./ -I./include

install:

	if [ ! -d /usr/local/lib/mathimatic ]; then \
		sudo mkdir /usr/local/lib/mathimatic; \
	fi
	sudo cp libmathimatic.so /usr/local/lib/mathimatic/
	sudo cp libmathimatic.conf /etc/ld.so.conf.d/
	sudo ldconfig
clean:
	rm *.o math
	rm *.a *.so
