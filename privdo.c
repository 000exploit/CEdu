/***************************************************
* 
* Name: privdo
* Description: Utility to run a program as EUID
* Author: 000exploit
* Purpose: Exploring the possibility of increasing/
*          lowering program priviliges to run soft-
*          ware with different access rights.
*
***************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

const char helpmsg[] = "Usage:\n\tprivdo [command] [args]";

int main(int argc, char* argv[]) {
	if ((argc < 2) || strcmp(argv[1], "-h") == 0) {
		fprintf(stderr, "%s\n", helpmsg);
		return 1;
	}

	int uid = getuid();
	int euid = geteuid();

	if (uid == euid) {
		puts("Do \"chmod u+s privdo\" or change the user.");
		return 2;
	}

	int suid = setuid(euid);
	if (suid < 0) {
		perror("setuid");
	}

	struct passwd *pw = getpwuid(euid);
	if(pw == NULL) {
		perror("getpwuid");
	        return 3;
	} else {
		printf("Executing command %s as user %s with argument %s\n",
				argv[1], pw->pw_name, argv[2]);
	}

	int exec = execvp(argv[1], argv+1);
	perror(argv[1]);
	return exec;
}
