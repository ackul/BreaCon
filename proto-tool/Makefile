
all: busy.o proto-tool.o account.o
	g++ -o proto-tool busy.o proto-tool.o
	g++ -o account account.o -pthread

account.o:
	g++ --std=c++11 -c account.cpp

busy.o: busy.s busy.h
	nasm -o busy.o -f elf64 busy.s

proto-tool.o: proto-tool.cpp busy.h
	g++ --std=c++11 -c proto-tool.cpp

clean:
	rm -f *.o proto-tool account
