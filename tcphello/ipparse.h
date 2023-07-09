typedef struct ipstruct {
	char ipaddr[16];
	int port;
} ipstruct_t;

struct ipstruct* parseip(const char addr[]);
