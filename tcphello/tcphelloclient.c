#define _DEFAULT_SOURCE

#include "ipparse.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

const char helpmsg[] = "Usage:\n\ttcphelloclient [ip:port]";

int send_messages(int fd) {
	char buf[80];

	while (1) {
		memset(&buf, '\0', sizeof(buf));
		fflush(stdin);

		int ok = read(STDIN_FILENO, &buf, sizeof(buf));
		if (!ok) {
			perror("read");
			return 1;
		}

		printf("(%i) Client to server : %s", ok, buf);

		ok = write(fd, buf, sizeof(buf));
		if (!ok) {
			perror("write");
			return 1;
		}
		
		if (strncmp("exit", buf, 4) == 0) {
			puts("Exit command.");
			break;
		}
	}
	return 0;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		puts(helpmsg);
		return 1;
	}

	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) {
		perror("Failed to create socket");
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

	int connfd = connect(tcp_socket,
			(struct sockaddr*) &address,
			sizeof(address));
	if (connfd < 0) {
		perror("connect");
		return 1;
	} else {
		printf("Connected to %s:%i\n", ip->ipaddr, ip->port);
	}
	send_messages(tcp_socket);

	close(tcp_socket);

	return 0;
}
