/*
 * Write a program that lists the process ID and command name for all processes being run by the user named in the program’s command-line argument.
 * This can be done by inspecting the Name: and Uid: lines of all of the /proc/PID/status files on the system.
 * Walking through all of the /proc/PID directories on the system requires the use of readdir(3), which is described in Section 18.8.
 * Make sure your program correctly handles the possibility that a /proc/PID directory disappears between the time that the program determines that the directory exists and the time that it   tries to open the corresponding /proc/PID/status file.
*/

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include "tlpi_hdr.h"

char *
getcmdline(char *uid);


int
main(int argc, char *argv[])
{
    struct passwd *pwd;
    uid_t u;
    DIR *dirp;
    struct dirent *dp;
    char *endptr, *filename;
    FILE *file;
    char su[20];
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s username \n", argv[0]);
    dirp = opendir("/proc/");
    if (dirp == NULL) {
        errExit("opendir failed on '/proc/'");
    }
    pwd = getpwnam(argv[1]);
    if(pwd==NULL)
        return -1;
    u = pwd->pw_uid;
    sprintf(su,"%d",u);
    for (;;) {
        errno = 0;              /* To distinguish error from end-of-directory */
        dp = readdir(dirp);
        if (dp == NULL)
            break;

        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;               /* Skip . and .. */
	strtol(dp->d_name, &endptr, 10);      
    	if (*endptr != '\0')                /* only allow  numeric string */
            continue;
	filename = strdup("/proc/");
	strcat(filename, dp->d_name);
	strcat(filename, "/status");
	file = fopen(filename, "r");
	if ( file != NULL )
        {
            char line [ 128 ]; /* or other suitable maximum line size */
            while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
            {
                if(strncmp(line,"Uid:", 4)==0) 
		{
		    int readu;
		    sscanf(line, "%*s %d %*d", &readu); //Eg. "Uid:	1000	1000	1000	1000"
		    if(readu==u)
		    {
			char *cmdlin =  getcmdline(dp->d_name);
			if(cmdlin)
                            printf("%s %s\n", dp->d_name, cmdlin);
		    }
		}
            }
	}
        fclose ( file );
    }

    if (errno != 0)
        errExit("readdir");

    if (closedir(dirp) == -1)
        errMsg("closedir");


    return 0;
}


char *
getcmdline(char *pid)
{
    char * filename = strdup("/proc/");
    strcat(filename, pid);
    strcat(filename, "/cmdline");
    char *line = (char *) malloc(128);
    FILE *file = fopen(filename, "r");
    if(!file)
	return NULL;
    if( fgets(line, 128, file)!=NULL)
    {
        return line;
    }
    else
	return NULL;
}






