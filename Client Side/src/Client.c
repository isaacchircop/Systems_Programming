#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int main(void) {

	int socketfd = socket(AF_INET, SOCK_STREAM, 0);

	// Bind socket to an address

	struct sockaddr_in address;

	// Describe address properties

	address.sin_family = AF_INET;
	address.sin_port = htons(8080);
	address.sin_addr.s_addr = inet_addr("127.0.0.1");

	// Connect to socket using above described address

	int res = connect(socketfd, (struct sockaddr *) &address, sizeof(address));

	if (res < 0) {

		// ERROR

		printf("%s\n", strerror(res));
		return -1;

	}

	// Send a message to the server

	char* message = "This is a message from the client.";

	send(socketfd, message, strlen(message), 0);



	close(socketfd);

	return 0;

}
