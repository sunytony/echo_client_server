all: echo_client

echo_client: echo_client.o
	g++ -o echo_client echo_client.o -lpthread

echo_client.o: echo_client.cpp
	g++ -c -o echo_client.o echo_client.cpp

clean:
	rm -f echo_client *.o