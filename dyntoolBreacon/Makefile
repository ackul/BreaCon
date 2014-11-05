DYNINST_INCLUDE=/u/a/c/achink/dyn_build/include
DYNINST_LIB=/u/a/c/achink/dyn_build/lib
CFLAGS=-g
CC=g++

breacon: breacon.o
	        $(CC) breacon.o -L$(DYNINST_LIB) -ldyninstAPI -lelf -ldwarf -o breacon -std=c++0x

breacon.o: breacon.cpp
	        $(CC) -c $(CFLAGS) -I$(DYNINST_INCLUDE) breacon.cpp -std=c++0x

mutatee: mutatee.o
	    g++ mutatee.o -o mutatee -pthread

mutatee.o: mutatee.cpp
	    g++ -c mutatee.cpp -pthread

libbreacondelay:
	gcc -fPIC -c -Wall -Werror delayLib.c
	gcc -shared -o libbreacondelay.so delayLib.o
	
