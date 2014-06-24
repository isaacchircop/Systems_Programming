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
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>

struct response {

	int number;
	char message[25];

};
typedef struct response rsp;

int main(void) {

	// Set Up Listen Port for client connection

	pid_t pid;

	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd < 0) {

		printf("%s\n", strerror(errno));
		return -1;

	}

	// Bind a server address to a socket
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

	// Listening for connections

	printf ("Listening for connections ... \n");

	int listenErr = listen(socketfd, 2);
	if (listenErr < 0) {

		printf("%s\n", strerror(errno));
		return -1;

	}

	// Create a new client address variable
	struct sockaddr_in clientAddr;
	unsigned int addressLength = sizeof(struct sockaddr_in);

	int newSocket;
	int parentRead[2];
	int childRead[2];

	do {

		// Accept Connections from client address
		newSocket = accept(socketfd, (struct sockaddr*) &clientAddr, &addressLength);

		if (newSocket < 0) {

			printf("%s\n", strerror(errno));
			exit(-1);

		}

		// Create Pipe for parent child communication

		int pipeErr = pipe(parentRead);

		if(pipeErr < 0){

			printf("%s\n", strerror(errno));
			exit(-1);

		}

		dup2(parentRead, childRead);

		// Create new process:	Child Process handles client requests
		//						Parent Process handles other connections

		pid = fork();

		if (pid > 0) {

			close (childRead[0]);		// Close read fd for childRead pipe
			close (parentRead[1]);		// Close write fd for parentRead pipe

			fcntl (childRead[0], F_SETFL, O_NONBLOCK);
			fcntl (parentRead[1], F_SETFL, O_NONBLOCK);

			// Read any updates from children

			off_t fileOffset;
			int offsetRead = read (parentRead[0], &fileOffset, sizeof(fileOffset));

			if (offsetRead > 0) {

				printf ("Received Update\n");

				// Send update to all children

				int count;
				read (parentRead[0], &count, sizeof(count));

				char *buff = (char *)malloc(sizeof(count));
				read (parentRead[0], &buff, count);

				write (childRead[1], &fileOffset, sizeof(fileOffset));
				write (childRead[1], &count, sizeof(count));
				write (childRead[1], &buff, count);

			}

		}

	} while (pid > 0);

	if (pid == 0) {

		close (childRead[1]);			// Close write fd for childRead pipe
		close (parentRead[0]);			// Close read fd for parentRead pipe

		fcntl (childRead[0], F_SETFL, O_NONBLOCK);
		fcntl (parentRead[1], F_SETFL, O_NONBLOCK);

		fcntl (newSocket, F_SETFL, O_NONBLOCK);

		// Child Process is created whenever clients connects to server - requests for rmmap

		printf ("\nAccepted Connection from client %d ... \n", getpid());

		// **** Start of rmmap ****

		// Get file offset to start mapping from
		off_t offset;
		read(newSocket, &offset, sizeof(offset));

		// Get Pathname of file from where we are to map
		char *pathname = (char *)malloc(sizeof(char) * 1024);		//Max Pathname accepted is 1024 characters long
		read(newSocket, pathname, sizeof(char)*1024);

		// Try Opening File
		int fd = open(pathname, O_RDWR);
		if (fd < 0) {

			printf("%s\n", strerror(errno));
			exit(-1);

		}

		// Should be decided from protocol
		int numOfChars = 100;											// Number of characters to be read at once

		char *buf = (char *)malloc(sizeof(char) * numOfChars);

		// Seek offset and start reading from file
		lseek (fd, offset, SEEK_SET);

		while (read (fd, buf, numOfChars) > 0) {

			// Send read data to client
			write(newSocket, buf, numOfChars);

		}

		// Should be decided from protocol
		char *eof = "-1";

		// Inform client of end of mapping
		write (newSocket, eof, sizeof(eof));

		// Free any open buffers and close file descriptor
		close (fd);
		free(buf);

		// **** End of rmmap tackling ****

		int request = 0;

		// Accept client requests until memory is unmapped

		do {

			// Check for updates from parent process

			off_t fileOffset;
			int offsetRead = read (childRead[0], &fileOffset, sizeof(fileOffset));

			if (offsetRead > 0) {

				printf ("Update\n");

				int count;
				read (childRead[0], &count, sizeof(count));

				char *buff = (char *)malloc(sizeof(count));
				int bytesRead = read (childRead[0], &buff, count);

				// Send Update information to client

				write (newSocket, &fileOffset, sizeof(fileOffset));
				write (newSocket, &count, sizeof(count));
				write (newSocket, &buff, count);

				free (buff);

			} else {

				/// Read for client requests

				int clientRead = read (newSocket, &request, sizeof(request));

				if (clientRead > 0) {

					switch (request) {

						case 1:

							// Unmap and close connection

							printf ("Closing Connection with client %d...\n", getpid());
							close (newSocket);
							close (parentRead[1]);
							close (childRead[0]);
							kill (getpid(), 0);		// Perform Suicide
							break;

						case 2: {

							// **** Start of write request handling ****

							printf ("Write Request received\n");

							rsp resp;
							char *message;

							// Get details of Write Request
							off_t write_offset;
							read(newSocket, &write_offset, sizeof(write_offset));

							int count;
							read(newSocket, (int *)&count, sizeof(int));

							char *buff = (char *)malloc(sizeof(char) * count);
							read(newSocket, buff, sizeof(char)*count);

							// Open file with Write Access
							int fd = open(pathname, O_WRONLY);
							if (fd < 0) {

								printf("%s\n", strerror(errno));

								resp.number = -1;
								message = "Could Not Open File";
								strcpy (resp.message, message);

								write (newSocket, &resp, sizeof(resp));

								break;

							}

							// ** File Lock Handling **

							struct flock savelock, unlock;

							savelock.l_type = F_WRLCK;
							savelock.l_whence = SEEK_SET;
							savelock.l_start = offset + write_offset;
							savelock.l_len = count;

							unlock.l_type = F_UNLCK;
							unlock.l_whence = SEEK_SET;
							unlock.l_start = offset + write_offset;
							unlock.l_len = count;

							// Lock file segment that will be modified
							int x = fcntl (fd, F_SETLK, &savelock);

							if (x == -1) {

								resp.number = -1;
								message = "File is locked";
								strcpy (resp.message, message);

								write (newSocket, &resp, sizeof(resp));

							}

							// Perform write

							off_t fileOffset = offset + write_offset;

							lseek (fd, fileOffset, SEEK_SET);
							int bytes = write (fd, buff, count);

							// Inform parent of file update
							write (parentRead[1], &fileOffset, sizeof(fileOffset));
							write (parentRead[1], &count, sizeof(count));
							write (parentRead[1], buff, count);

							char c;
							scanf ("%c", &c);

							// Unlock file segment that was modified
							int y = fcntl (fd, F_SETLK, &unlock);

							if (y == -1) {

								resp.number = -1;
								message = "Could not unlock file";
								strcpy (resp.message, message);

								write (newSocket, &resp, sizeof(resp));

							} else {

								if (bytes == -1) {

									printf ("%s\n", strerror(errno));

								} else {

									resp.number = bytes;
									message = "Success";
									strcpy (resp.message, message);

									write (newSocket, &resp, sizeof(resp));

								}

							}

							// Free buffer and close file descriptor
							free (buff);
							int k = close (fd);

							printf ("%d\n", k);

							break;

						}

					}

				}

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
