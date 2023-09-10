#/bin/bash
libsrc=$(wildcard ./src/*.cpp)
libobj=$(patsubst %.cpp, %.o, $(libsrc))

testsrc=$(wildcard ./test/*.cpp)
testobj=$(patsubst %.cpp, %.o, $(testsrc))

testutilsrc=$(wildcard ./testutil/*.cpp)
testutilobj=$(patsubst %.cpp, %.o, $(testutilsrc))

#1.Define the targets
target:lib app

app: math jatool
math: $(testobj) $(libobj)
#	g++ -o math $(testobj) -L. -lmathimatic
	g++ -o math $(testobj) $(libobj)

jatool: $(testutilobj) $(libobj)
	#	g++ -o math $(testobj) -L. -lmathimatic
	g++ -o jatool $(testutilobj) $(libobj)


#2.Define the lib related part
lib: dot_so dot_a
dot_so:
	g++ $(libsrc) -fPIC -shared -o libmathimatic.so -I./ -I./include -I./util
dot_a: $(libobj)
	ar rc libmathimatic.a $(libobj)

%.o:%.cpp
	g++ -o $@ -c $< -fpermissive -Wattributes -I./ -I./include -I./util

#3. Define Installation for this lib
install:
	if [ ! -d /usr/local/lib/mathimatic ]; then \
		sudo mkdir /usr/local/lib/mathimatic; \
	fi
	sudo cp libmathimatic.so /usr/local/lib/mathimatic/
	sudo cp libmathimatic.conf /etc/ld.so.conf.d/
	sudo ldconfig

clean:
	rm $(libobj) $(testobj) $(testutilobj) math jatool
	rm *.a *.so
