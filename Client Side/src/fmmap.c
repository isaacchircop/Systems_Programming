#ifndef FMMAP
#define FMMAP

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/mman.h>
#include <unistd.h>

#include "fmmap.h"

#include <stdlib.h>
#include <string.h>

// Type definition for remote file location
struct fileloc {
    struct in_addr ipaddress; // Remote IP
    int port;                 // Remote port
    char *pathname;           // Remote file to be memory mapped
};
typedef struct fileloc fileloc_t;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

// Server Information
int socketfd;
struct sockaddr_in serverAddr;

// Memory map remote file
void *rmmap(fileloc_t location, off_t offset)
{

	void *MAPFAILED = (void*)-1;

	printf ("Connecting to Server...\n");

	// Creating socket and binding server address to it

	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr = location.ipaddress;
	serverAddr.sin_port = location.port;

	int connSucc = connect(socketfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	if (connSucc < 0) {
		printf ("Error Establishing Server Connection");
		return MAPFAILED;

	}

	printf ("Successfully Connected!\n");

	// Connection Established.  Send request to server.

	printf ("Sending request to server...\n");

	int w1 = write (socketfd, (off_t *)&offset, sizeof(offset));
	int w2 = write (socketfd, location.pathname, strlen(location.pathname));

	// Should be decided from protocol

	int numOfChars = 100;
	char *eof = "-1";

	char *data = (char *)malloc(sizeof(char) * numOfChars);
	char *buf = (char *)malloc(sizeof(char) * numOfChars);

	while (read(socketfd, buf, numOfChars) > 0  && *buf != *eof) {

		// Successful Read from Server

		if (strlen(data) + strlen(buf) > sizeof(data)) {

			data = realloc(data, sizeof(data) + numOfChars);

		}

		strcat (data, buf);

	}

	free (buf);

	return data;

}
int rmunmap(void *addr)
{

	int request = 1;
	write (socketfd, &request, sizeof(request));

	free (addr);
	return close (socketfd);

}

ssize_t mread(void *addr, off_t offset, void *buff, size_t count)
{

	memcpy (buff, addr+offset, count);

	return count;

}

ssize_t mwrite(void *addr, off_t offset, void *buff, size_t count)
{

	int mapSize = strlen((char *)addr);

	if ((offset <= 0) || (offset > mapSize)) {

		return 0;

	}

	// Reallocating addr if necessary

	// If last address - offset is smaller than expected no. of bytes to be written reallocate.

	/*
	if (((addr + mapSize) - (addr + offset)) < count) {

		printf ("Prev Size %d\n", (int)sizeof(addr));
		addr = realloc(addr, sizeof(addr) + count);
		printf ("New Size %d\n", (int)sizeof(addr));

	}

	*/

	int i = 0;

	int size = count / sizeof(char);

	for (i = 0; i < size; i++) {

		((char *)addr)[offset + i] = ((char *)buff)[i];

	}

	return i;

}

#endif
