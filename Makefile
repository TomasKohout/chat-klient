CC = g++
CCFLAGS = -Wall

chat-klient: chat-klient.o
	$(CC) $(CCFLAGS) -o chat-klient chat-klient.o -pthread

chat-klient.o: chat-klient.cpp
	$(CC) $(CCFLAGS) -c chat-klient.cpp -pthread

clean:
	- rm *.o
	- rm example
