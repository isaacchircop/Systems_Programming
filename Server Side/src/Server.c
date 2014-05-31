#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

typedef struct packet packet;

int main(void) {

	// Create a new Socket


	printf ("Creating Socket ... \n");

	int socketfd = socket(AF_INET, SOCK_STREAM, 0);

	if (socketfd < 0) {

		// Error Returned

		printf("%s", strerror(socketfd));
		return -1;

	}

	// Bind a server address to a socket

	printf ("Binding Address ... \n");

	struct sockaddr_in serverAddr;

	bzero(&serverAddr, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(5000);

	int bindErr = bind(socketfd, (struct sockaddr*) &serverAddr, sizeof(serverAddr));

	if (bindErr < 0) {

		// Error Returned

		printf("%s", strerror(bindErr));
		return -1;

	}

	printf ("Binding Successful ... \n");

	// Create a listen connection - 5 represents number of clients in queue
	// awaiting accept from server

	int listenErr = listen(socketfd, 2);

	if (listenErr < 0) {

		// Error Returned

		printf("%s", strerror(listenErr));
		return -1;

	}

	printf ("Listening to connections...\n");


	// Accepting a connection from a client

	// Create a new client address variable

	struct sockaddr_in clientAddr;
	unsigned int addressLength = sizeof(struct sockaddr_in);

	// Accept Connections from client address

	int newSocket = accept(socketfd, (struct sockaddr*) &clientAddr, &addressLength);

	if (newSocket < 0) {

		// Error Returned

		printf("%s", strerror(newSocket));

	}

	printf ("Accepted Connection...\n");

	off_t offset;
	int r1 = read(newSocket, &offset, sizeof(offset));

	char *pathname = (char *)malloc(sizeof(char) * 1024);		//Max Pathname accepted is 1024 characters long
	int r2 = read(newSocket, pathname, sizeof(char)*1024);

	FILE *fp = fopen(pathname, "r");

	if (fp == NULL) {

		printf ("Exiting");
		exit(-1);

	}

	free (pathname);

	// Should be decided from protocol
	int numOfChars = 100;											// Number of characters to be read at once

	char *buf = (char *)malloc(sizeof(char) * numOfChars);

	while (fgets(buf, numOfChars, fp) != NULL) {

		// Successful Read from file - Send to client

		int w1 = write(newSocket, buf, numOfChars);

	}

	fclose(fp);
	free(buf);

	close (newSocket);
	close (socketfd);

	return 0;

}
