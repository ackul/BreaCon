INC_PATH=/u/a/c/achink/dyn_build/include
LIB_PATH=/u/a/c/achink/dyn_build/lib
DYN_LIBS=-ldyninstAPI -lelf -ldwarf
DYN_OBJS=parse-args.o printer.o log.o mutatee.o trial-run.o utils.o random.o
RT_SRCS=dynamo-rt.cpp random.cpp log.cpp
CFLAGS=--std=c++11
CC=/afs/cs.wisc.edu/s/gcc-4.9.1/amd64_rhel6/bin/g++
OC=g++
OCFLAGS=--std=c++0x
BU=./build-up

dynamo: $(BU) $(DYN_OBJS) libdynamo-rt.so
	$(BU) version.h
	$(CC) $(CFLAGS) -o dynamo dynamo.cpp $(DYN_OBJS) -L$(LIB_PATH) $(DYN_LIBS)

libdynamo-rt.so: $(BU) $(RT_SRCS)
	$(BU) version.h
	$(CC) $(CFLAGS) -fPIC -shared -o libdynamo-rt.so $(RT_SRCS)

log.o: log.h log.cpp
	$(CC) $(CFLAGS) -c $*.cpp

mutatee.o: mutatee.h mutatee.cpp log.h
	$(OC) $(OCFLAGS) -c $*.cpp -I$(INC_PATH)

parse-args.o: parse-args.h parse-args.cpp log.h
	$(CC) $(CFLAGS) -c $*.cpp

printer.o: printer.h printer.cpp version.h
	$(CC) $(CFLAGS) -c $*.cpp

random.o: random.h random.cpp
	$(CC) $(CFLAGS) -c $*.cpp

trial-run.o: trial-run.h trial-run.cpp mutatee.h log.h utils.h
	$(CC) $(CFLAGS) -c $*.cpp

utils.o: utils.h utils.cpp log.h
	$(CC) $(CFLAGS) -c $*.cpp

version.o: version.h version.cpp
	$(CC) $(CFLAGS) -c $*.cpp

build-up: build-up.cpp
	$(CC) $(CFLAGS) -o $(BU) build-up.cpp

clean:
	rm *.so *.o dynamo build-up
