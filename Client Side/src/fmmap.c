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

// Packet of data structure
struct packet {

	int type;
	char* data;
	int length;

};
typedef struct packet packet;

// Memory map remote file
void *rmmap(fileloc_t location, off_t offset)
{

	printf ("Connecting to Server...\n");

	// Creating socket and binding server address to it

	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr = location.ipaddress;
	serverAddr.sin_port = location.port;

	int connSucc = connect(socketfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	if (connSucc < 0) {
		printf ("Error Establishing Server Connection");
		exit(-1);
	}

	printf ("Successfully Connected!\n");

    // Connection Established.  Send request to server.

	printf ("Sending request to server...\n");

	printf ("%d", (int)strlen(location.pathname));

	write (socketfd, location.pathname, strlen(location.pathname));
	write (socketfd, (off_t *)&offset, sizeof(offset));

	// Read packet of data from server

	int size;
	read (socketfd, &size, sizeof(size));

	printf ("Read size...\n");

	char *memoryMap = (char *)malloc(size);
	read (socketfd, memoryMap, size);

	printf ("Before return");

	return memoryMap;

}
/*
int rmunmap(void *addr)
{

	return munmap(addr,length);

	int value;

    if((value = munmap(addr, length)) == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

ssize_t mread(void *addr, off_t offset, void *buff, size_t count)
{
    int value;

    if((value = read((addr + offset),buff,count)) == 0)
    {
        return count;
    }
    else
    {
        return -1;
    }
}

ssize_t mwrite(void *addr, off_t offset, void *buff, size_t count)
{
    int value;

    if((value = write(addr + offset,buff,count)) == 0)
    {
        return count;
    }
    else
    {
        return -1;
    }

}*/

#endif
