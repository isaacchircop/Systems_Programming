#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

int main(void) {

	// Create a new Socket

	int socketfd = socket(AF_INET, SOCK_STREAM, 0);

	if (socketfd < 0) {

		// Error Returned

		printf("%s", strerror(socketfd));
		return -1;

	}

	printf("Socket created...\n");




	// Bind a server address to a socket

	// Create new socket address

	struct sockaddr_in serverAddr;

	// Fill memory space with zeros
	bzero(&serverAddr, sizeof(serverAddr));

	// Define protocol family
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	// Define port number
	serverAddr.sin_port = htons(8080); //Port Number less than 1024 requires super user access

	int bindErr = bind(socketfd, (struct sockaddr*) &serverAddr, sizeof(serverAddr));

	if (bindErr < 0) {

		// Error Returned

		printf("%s", strerror(bindErr));
		return -1;

	}

	printf ("Server address bound to socket...\n");




	// Create a listen connection - 5 represents number of clients in queue
	// awaiting accept from server

	int listenErr = listen(socketfd, 5);

	if (listenErr < 0) {

		// Error Returned

		printf("%s", strerror(listenErr));
		return -1;

	}

	printf ("Listening to connections...\n");




	// Accepting a connection from a client

	// Create a new client address variable

	struct sockaddr_in clientAddr;
	int addressLength = sizeof(struct sockaddr_in);

	// Accept Connections from client address

	int newSocket = accept(socketfd, (struct sockaddr*) &clientAddr, &addressLength);

	if (newSocket < 0) {

		// Error Returned

		printf("%s", strerror(newSocket));

	}

	printf ("Accepted Connection...\n");




	// New Socket created for successfully connected client

	// Read messages sent by client

	char* message = malloc(sizeof(char) * 1024);

	printf ("Commencement of Receiving process...\n");

	while (recv(newSocket, message, strlen(message), 0) > 0) {

		// Output received data
		printf ("entered loop...\n");

		printf ("SERVER: %s", message);

	}

	free(message);

	printf ("End of Receiving process...\n");

	int closeNewSocket = close(newSocket);
	int closeInitSocket = close(socketfd);

	if (closeNewSocket < 0) {

		// Error Returned

		printf("%s", strerror(closeNewSocket));
		return -1;

	}

	if (closeInitSocket < 0) {

		// Error Returned

		printf("%s", strerror(closeInitSocket));
		return -1;

	}

	printf ("Sockets Closed...");

	return 0;

}
