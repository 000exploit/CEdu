#define _DEFAULT_SOURCE

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include "ipparse.h"

const char helpmsg[] = "Usage:\n\ttcphelloserver [ip:port]";

int print_messages(int fd) {
	char buf[80];

	while (1) {
		memset(&buf, '\0', sizeof(buf));

		int ok = read(fd, &buf, sizeof(buf));
		if (ok < 0) {
			perror("read");	
			return 1;
		} else if (ok == 0) {
			puts("No data.");
			return 2;
		}

		printf("(%i) Client to server : %s", ok, buf);

		if (strncmp("exit", buf, 4) == 0) {
			puts("Exit command.");
			return 0;
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		puts(helpmsg);
		return 1;
	}

	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) {
		perror("TCP socket creation failed");
		return 1;
	}

	ipstruct_t* ip = parseip(argv[1]);
	if (!ip) {
		close(tcp_socket);
		fprintf(stderr, "Incorrect string: %s\n", argv[1]);
		return 1;
	}

	struct sockaddr_in address;	
	address.sin_family = AF_INET;
	int ok = inet_aton(ip->ipaddr, &(address.sin_addr));
	if (!ok) {
		close(tcp_socket);
		fprintf(stderr, "IP address %s is incorrect.\n", ip->ipaddr);
		return 1;
	}
	address.sin_port = htons(ip->port);

	ok = bind(tcp_socket, (struct sockaddr*) &address,
			sizeof(address));
	if (ok != 0) {
		close(tcp_socket);
		perror("Binding address failed");
		return 1;
	} else
		puts("Socket successfully binded.");

	int listening = listen(tcp_socket, 5);
	if (listening != 0) {
		close(tcp_socket);
		perror("Listening failed");
		return 1;
	} else
		printf("Server is listening on port %i...\n", ip->port);

	socklen_t len = sizeof(address);
	int connfd = accept(tcp_socket, (struct sockaddr*) &address,
			&len);
	puts("Got connection.");

	print_messages(connfd);

	close(tcp_socket);

	return 0;
}
