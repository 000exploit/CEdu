/***************************************************
* 
* Name: playpause
* Description: Utility to stop and continuing the
*              execution of processes
* Author: 000exploit
* Purpose: Experimenting with signals and using
* 	   procfs to get information about processes
*
***************************************************/

#define _POSIX_C_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/sendfile.h>

#define PROCFS_PATH "/proc/"
#define PROCFS_STATFILE "/stat"
#define BUF_SIZE 32

const char help[] = "Usage:\n\tplaypause [pid]";

char read_state(char path[]) {
	int avail = access(path, R_OK | F_OK);
	if (avail < 0) {
		perror(path);
		return 2;
	}

	int pidfd = open(path, O_RDONLY);
	if (pidfd < 0) {
		perror("open");
		return 2;
	}

	char buf[BUF_SIZE] = {0};
	int count = 0;
	char pidstate = '\0';
	while (!pidstate) {
		int readval = read(pidfd, &buf, sizeof(buf));
		if (readval < 0) {
			perror("Read from stat file error");
			return 3;
		}
		for(int i = 0; i < sizeof(buf); i++) {
			if (buf[i] == 32)
				count++;
			else if (i < (sizeof(buf)-1)
					&& buf[i+1] == 32
					&& count == 2)
			{
				pidstate = buf[i];
				break;
			}
		}
	};
	return pidstate;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		puts(help);
		return 1;
	}

	char* fullpath = malloc((sizeof(PROCFS_PATH) + sizeof(PROCFS_STATFILE)
				+ strlen(argv[1]) + 1) * sizeof(char));

	sprintf(fullpath, "%s%s%s", PROCFS_PATH, argv[1], PROCFS_STATFILE);

	char pidstate = read_state(fullpath);
	free(fullpath);
	if (pidstate < 48 || pidstate == '\0')
		return pidstate;

	int killret = 0;
	if (pidstate == 'R' || pidstate == 'S') {
		killret = kill(atoi(argv[1]), SIGSTOP);
	} else {
		killret = kill(atoi(argv[1]), SIGCONT);
	}
	if (killret < 0) {
		perror("playpause");
		return 4;
	}

	return 0;
}
