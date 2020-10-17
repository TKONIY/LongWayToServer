CC = g++ -std=c++11 -pthread
all: build/server build/client

build/server: build/server.o
	$(CC) build/server.o -o build/server
build/client: build/client.o
	$(CC) build/client.o -o build/client
build/server.o: src/server.cpp
	$(CC) -c src/server.cpp -o build/server.o 
build/client.o: src/client.cpp 
	$(CC) -c src/client.cpp -o build/client.o

.PHONY:
clean:
	rm -f build/*