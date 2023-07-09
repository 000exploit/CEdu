#include "ipparse.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct ipstruct* parseip(const char addr[]) {
	struct ipstruct* ip = malloc(sizeof(struct ipstruct));

	char* token;
	char* delim = ":";
	token = strtok((char*) addr, delim);
	if (token != NULL)
		strcpy(ip->ipaddr, token);
	else
		return NULL;
	token = strtok(NULL, delim);
	ip->port = atoi(token);

	return ip;
}
