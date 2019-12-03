all: echo_server

echo_server: echo_server.o
	g++ -o echo_server echo_server.o -lpthread

echo_server.o: echo_server.cpp
	g++ -c -o echo_server.o echo_server.cpp

clean:
	rm -f echo_server *.o