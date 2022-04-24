

client: client.o
	g++ -o client client.o 

server: server.o
	g++ -o server server.o -lpthread

client.o: client.cpp
	g++ -c client.cpp  

server.o: server.cpp
	g++ -c server.cpp -lpthread 


.PHONY: clean client server


clean:
	rm -f *.o server client
