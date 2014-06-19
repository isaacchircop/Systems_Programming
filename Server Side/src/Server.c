#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

int main(void) {

	pid_t pid;

	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd < 0) {

		printf("%s\n", strerror(errno));
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

		printf("%s\n", strerror(errno));
		return -1;

	}

	printf ("Binding Successful ... \n");

	// Listening for connections

	int listenErr = listen(socketfd, 2);
	if (listenErr < 0) {

		printf("%s\n", strerror(errno));
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

			printf("%s\n", strerror(errno));
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
		read(newSocket, &offset, sizeof(offset));

		char *pathname = (char *)malloc(sizeof(char) * 1024);		//Max Pathname accepted is 1024 characters long
		read(newSocket, pathname, sizeof(char)*1024);

		int fd = open(pathname, O_RDONLY);
		if (fd < 0) {

			printf("%s\n", strerror(errno));
			exit(-1);

		}

		// Should be decided from protocol
		int numOfChars = 100;											// Number of characters to be read at once

		char *buf = (char *)malloc(sizeof(char) * numOfChars);

		lseek (fd, offset, SEEK_SET);

		while (read (fd, buf, numOfChars) > 0) {

			// Successful Read from file - Send to client
			write(newSocket, buf, numOfChars);

		}

		char *eof = "-1";

		write (newSocket, eof, sizeof(eof));

		close (fd);
		free(buf);

		// End of rmmap tackling

		int request = 0;

		do {

			// Read client requests

			read (newSocket, &request, sizeof(request));

			switch (request) {

			case 1:
				printf ("Closing Connection for client...\n");
				close (newSocket);
				break;

			case 2:

				fd = open(pathname, O_WRONLY);

				if (fd < 0) {

					printf("%s", strerror(errno));
					exit(-1);

				}

				off_t offset;
				read(newSocket, &offset, sizeof(offset));

				lseek (fd, offset, SEEK_SET);

				int count;
				read(newSocket, (int *)&count, sizeof(int));

				char *buff = (char *)malloc(sizeof(char) * count);		//Max Pathname accepted is 1024 characters long
				read(newSocket, buff, sizeof(char)*count);

				write (fd, buff, count);

				close(fd);

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
