.PHONY: all clean

all: clean server client
#test
#test: TestCounter.o Test.o
#	gcc test.o -o test
#test.o: TestCounter.cpp Test.cpp
#	gcc -c test.cpp
server: Stack.hpp server.o
	gcc server.o -o server
client: client.o Stack.hpp
	gcc client.o -o client
server.o: server.cpp Stack.hpp
	gcc -c server.cpp
client.o: client.cpp Stack.hpp
	gcc -c client.cpp

clean :
	rm -f *.o server client file.txt