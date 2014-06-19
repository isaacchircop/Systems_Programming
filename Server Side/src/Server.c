#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

typedef struct packet packet;

int main(void) {

	pid_t pid;

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

	// Listening for connections

	int listenErr = listen(socketfd, 2);
	if (listenErr < 0) {

		// Error Returned
		printf("%s", strerror(listenErr));
		return -1;

	}

	// Create a new client address variable
	struct sockaddr_in clientAddr;
	unsigned int addressLength = sizeof(struct sockaddr_in);

	int newSocket;

	do {

		// Accept Connections from client address
		newSocket = accept(socketfd, (struct sockaddr*) &clientAddr, &addressLength);

		if (newSocket < 0) {

			// Error Returned
			printf("%s", strerror(newSocket));
			exit(-1);

		}

		// Create new process:	Child Process handles client requests
		//						Parent Process handles other connections

		pid = fork();

	} while (pid > 0);

	if (pid == 0) {

		// Child Process is created whenever clients connects to server - requests for rmmap

		printf ("Accepted Connection...\n");

		// Tackling rmmap request

		off_t offset;
		int r1 = read(newSocket, &offset, sizeof(offset));

		char *pathname = (char *)malloc(sizeof(char) * 1024);		//Max Pathname accepted is 1024 characters long
		int r2 = read(newSocket, pathname, sizeof(char)*1024);

		FILE *fp = fopen(pathname, "r");

		if (fp == NULL) {

			printf ("Exiting\n");
			exit(-1);

		}

		free (pathname);

		// Should be decided from protocol
		int numOfChars = 100;											// Number of characters to be read at once

		char *buf = (char *)malloc(sizeof(char) * numOfChars);

		fseek(fp, offset, SEEK_SET);

		while (fgets(buf, numOfChars, fp) != NULL) {

			// Successful Read from file - Send to client

			int w1 = write(newSocket, buf, numOfChars);

		}

		char *eof = "-1";

		write (newSocket, eof, sizeof(eof));

		fclose(fp);
		free(buf);

		// End of rmmap tackling

		int request = 0;

		do {

			// Read client requests

			read (newSocket, &request, sizeof(request));

			switch (request) {

			case 1:
				printf ("rmunmap\n");
				close (newSocket);
				break;

			case 2:
				printf ("write\n");
				break;

			}

		} while (request != 1);

		kill(getpid(), SIGTERM);

	} else if (pid < 0) {

		// Error forking
		return -1;

	}

	close (socketfd);

	return 0;

}
