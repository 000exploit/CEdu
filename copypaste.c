/***************************************************
* 
* Name: copypaste
* Description: Utility for copying and pasting files
* Author: 000exploit
* Purpose: Exploring the possibilities of read() and
* 	   write() syscalls as tool for duplicating
* 	   information.
*
***************************************************/

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


/*
* EXIT CODE CONVENTION
* 0 - success
* 1 - params error
* 2 - I/O error
*/

const char helpmsg[] = "Usage:\n\tcopypaste [src] [dest]";

int bufrw(int inputfd, int outputfd, char* buf, int size) {
	int readval = read(inputfd, buf, size);
	if (readval < 0) {
ioerr:		perror("I/O error");
		return 2;
	} else if (readval == 0) {
		perror("Unexpected EOF");
		return 2;
	}

	int writeval = write(outputfd, buf, size);
	if (writeval < 0) {
		goto ioerr;
	}
	return 0;
}

int copyfile(char* src, char* dest)
{
	int srcfd = open(src, O_RDONLY);
	int destfd = open(dest, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);

	if (!srcfd) {
		fprintf(stderr, "Failed to open \"%s\".", src);
		return 1;
	}
	if (!destfd) {
		fprintf(stderr, "Failed to open \"%s\".", dest);
		return 1;
	}

	struct stat srcstat = {0};
	int statret = fstat(srcfd, &srcstat);
	if (statret < 0)
		return 2;

	printf("File size: %li bytes\n", srcstat.st_size);

	int wstatus = 0;
	int pid = fork();
	if (pid == 0) {
#ifdef __unix__
		struct statvfs fsstat = {0};
		if ((fstatvfs(destfd, &fsstat)) == 0) {
			if (fsstat.f_bavail < srcstat.st_size) {
				fprintf(stderr,\
					"Insufficient disk space. Requested: %li, available: %li\n",\
					srcstat.st_size, fsstat.f_bavail);
				return 2;
			}
		}
#if defined(__linux__) && defined(ZEROCOPYV1)
#include <sys/sendfile.h>
		int sendval = sendfile(destfd, srcfd, 0, srcstat.st_size);
		if (sendval < 0) {
			perror("I/O error");
			return 2;
		}
		fprintf(stderr, "Total write: %i bytes\n", sendval);
#else
		int page_size = sysconf(_SC_PAGE_SIZE);
		int avail_pages = -1;
#ifdef __linux__
		avail_pages = sysconf(_SC_AVPHYS_PAGES);

#elif __freebsd__
#include <sys/sysctl.h>
		int sysctlreq[2] = {CTL_HW, HW_AVAILPAGES};
		size_t len = sizeof(sysctlreq);
		int sysctlret = sysctl(sysctlreq, 2, &avail_pages, &len, NULL, 0);
		if (sysctlreq < 0) {
			perror("sysctl");
			avail_pages = -1;
		}
#endif
		char* buf;
		unsigned long free_memory = 0;
		if (avail_pages != -1) {
			free_memory = page_size * avail_pages;
			if (free_memory < (unsigned long)(srcstat.st_size - LONG_MAX))
				buf = malloc(free_memory);
		} else {
				buf = malloc(sizeof(char)*srcstat.st_size);
				free_memory = 0;
		}
		if (buf == NULL) {
			perror("Critical error");
			return 2;
		}
		
		if (!free_memory) {
			int tail = srcstat.st_size % free_memory;
			int total = (srcstat.st_size - tail) / free_memory;
		
			for (int i = 0; i < total; i++) {
				int bufrwret = bufrw(srcfd, destfd, buf, total);
				if (bufrwret != 0)
					return 2;
			}
			if (tail > 0) {
				bufrw(srcfd, destfd, buf, tail);
			}
		} else {
			int bufrwret = bufrw(srcfd, destfd, buf, srcstat.st_size);
			if (bufrwret != 0)
				return 2;
		}

		free(buf);
#endif
#else
#error "Only UNIX-like systems is supported."
#endif
		exit(0);
	}
	waitpid(0, &wstatus, 0);
	close(srcfd);
	close(destfd);
	if (WIFEXITED(wstatus))
		return 2;

	return 0;
}

int main(int argc, char* argv[])
{
	if (argc < 3) {
		printf("%s\n", helpmsg);
		return 1;
	}
	
	int src = access(argv[1], F_OK | R_OK);
	//int dest = access(argv[2], W_OK);
	if (src != 0)
	{
		fputs("Invalid source file path.\n", stdout);
		return 1;
	}

	int ret = copyfile(argv[1], argv[2]);

	return ret;
}
