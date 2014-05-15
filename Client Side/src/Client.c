#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "fmmap.h"

int main(void) {

	struct in_addr address;
	address.s_addr = inet_addr("127.0.0.1");

	fileloc_t file;
	file.ipaddress = address;
	file.pathname = "/home/isaac/Test.txt";
	file.port = htons(5000);

	char* memoryMap = rmmap(file, 0);

	printf ("Data Received: %s", memoryMap);

	return 0;

}
