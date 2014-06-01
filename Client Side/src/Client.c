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
	file.pathname = "/home/isaac/Test.txt";		//20bytes
	file.port = htons(5000);

	char* memoryMap = rmmap(file, 5);
	printf ("Mapped Data: \n%s", memoryMap);

	char *buf = (char *)malloc(sizeof(char));
	int y = mwrite(memoryMap, 0, buf, 10);
	int z = mread(memoryMap, 0, buf, 10);

	int x = rmunmap(memoryMap);
	printf ("Mapped Data: \n%s", memoryMap);

	return 0;

}
