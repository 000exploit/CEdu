/***************************************************
* 
* Name: randpass
* Description: Utility for generating passwords
* Author: 000exploit
* Purpose: Experimenting with the ability to use the
*          least number of syscalls.
*
***************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/random.h>

#define NUMBERS(NUM) (NUM >= 42 && NUM <= 57)
#define UPPERCASE(NUM) (NUM >= 63 && NUM <= 90)
#define LOWERCASE(NUM) (NUM >= 97 && NUM <= 122)
#define BUF_MULTIPLIER 2
#define BUF_SIZE(LEN) (LEN*BUF_MULTIPLIER)
#define WRITEABLE(LEN) (LEN*BUF_MULTIPLIER-1)

const char helpmsg[] = "Usage: randpass [length]";

int main(int argc, char* argv[]) {
	if (argc < 2) {
		puts(helpmsg);
		exit(EXIT_FAILURE);
	}

	errno = 0;
	unsigned long len = strtoul(argv[1], NULL, 10);
	if (len == 0 || errno) {
		perror("strtol");
		exit(EXIT_FAILURE);
	}

	if (len > 100000000) {
		fprintf(stderr, "Number is too big.\n");
		exit(2);
	}

	char* buf = calloc(sizeof(char), len*2);
	char* pass = malloc(sizeof(char)*len);

	int j = 0;
	while (j < len) {
		arc4random_buf(buf, WRITEABLE(len));
		for (int i = 0; i < WRITEABLE(len) && j < len; i++)
			if (NUMBERS(buf[i]) || UPPERCASE(buf[i]) || LOWERCASE(buf[i])) {
				pass[j] = buf[i];
				++j;
			}
	}
	buf[len] = '\0';

	printf("Your password: %s\n", pass);

	free(buf);
	return 0;
}
