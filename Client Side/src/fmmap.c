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
#include <errno.h>

struct fileloc {
    struct in_addr ipaddress; // Remote IP
    int port;                 // Remote port
    char *pathname;           // Remote file to be memory mapped
};
typedef struct fileloc fileloc_t;

struct response {

	int number;
	char message[25];

};
typedef struct response rsp;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

// Server Information
int socketfd;
struct sockaddr_in serverAddr;

// File Offset
off_t fileoffset;

// Memory map remote file
void *rmmap(fileloc_t location, off_t offset)
{

	void *MAPFAILED = (void*)-1;

	// Creating socket and binding server address to it

	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr = location.ipaddress;
	serverAddr.sin_port = location.port;

	int connSucc = connect(socketfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	if (connSucc < 0) {
		printf("%s\n", strerror(errno));
		return MAPFAILED;

	}

	printf ("Connection Established!\n");

	// Connection Established.  Send request to server.

	write (socketfd, (off_t *)&offset, sizeof(offset));
	write (socketfd, location.pathname, strlen(location.pathname));

	fileoffset = offset;

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

	printf ("Memory map Successful\n");

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

	int succRead = 1;

	while (succRead > 0) {

		off_t updateOffset;
		succRead = read (socketfd, &updateOffset, sizeof(updateOffset));

		if (succRead > 0) {

			// Read remaining update details

			int count;
			read (socketfd, &count, sizeof(count));

			char *updatebuff = (char *)malloc(count);
			read (socketfd, updatebuff, count);

			// Update information

			off_t memoryOffset = updateOffset - fileoffset;

			if (memoryOffset < 0) {

				// Error

			} else {

				memcpy (addr + memoryOffset, updatebuff, count);

			}

		}

	}

	// After update return read data

	memcpy (buff, addr+offset, count);

	return count;

}

ssize_t mwrite(void *addr, off_t offset, void *buff, size_t count)
{

	// Send Request to server
	int request = 2;
	write (socketfd, &request, sizeof(request));

	// Check that offset is valid
	int mapSize = strlen((char *)addr);
	if ((offset < 0) || (offset > mapSize)) {

		printf ("Invalid Offset");
		return -1;

	} else {

		// **** Try Updating File On Server ****

		// Send Write Details
		write (socketfd, (off_t *)&offset, sizeof(offset));
		write (socketfd, (int *)&count, sizeof(int));
		write (socketfd, buff, count);

		// Get Server Response about write
		rsp resp;
		read (socketfd, &resp, sizeof(resp));

		if (resp.number == -1) {

			// Error
			printf ("%s\n", resp.message);
			return -1;

		} else {

			// Update Local Version

			memcpy (addr + offset, buff, count);
			return count;

		}

	}

}

#endif
