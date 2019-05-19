/*
 * Write a program that lists all processes that have a particular file pathname open.
 * This can be achieved by inspecting the contents of all of the /proc/PID/fd/ symbolic links. 
 * This will require nested loops employing readdir(3) to scan all /proc/PID directories, 
 * and then the contents of all /proc/PID/fd entries within each /proc/PID directory.
 * To read the contents of a /proc/PID/fd/n symbolic link requires the use of readlink()
 * */

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>
#include "tlpi_hdr.h"

char *
getprocessname(char *pid);


int
main(int argc, char *argv[])
{
    DIR *dirp, *dirf;
    struct dirent *dp, *dpf;
    char *endptr, *fdirname, pathname[PATH_MAX];
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname \n", argv[0]);
    dirp = opendir("/proc/");
    if (dirp == NULL) {
        errExit("opendir failed on '/proc/'");
    }
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
	fdirname = strdup("/proc/");
	strcat(fdirname, dp->d_name);
	strcat(fdirname, "/fd/");
	//printf("%s\n", fdirname);
	dirf = opendir(fdirname);
	if (dirf == NULL) {
		continue;
        	errExit("opendir failed on '/proc/PID/fd'");
    	}
	for(;;)
	{
	    dpf = readdir(dirf);
	    if(dpf==NULL)
 	        break;
	    if (strcmp(dpf->d_name, ".") == 0 || strcmp(dpf->d_name, "..") == 0)
            	continue;               /* Skip . and .. */
	    char *filepath = strdup(fdirname);
	    strcat(filepath, dpf->d_name);
	    int len = readlink(filepath, pathname, PATH_MAX);
	    pathname[len] = '\0';
	    if(strncmp(pathname, argv[1],len)==0)
	    	printf("%s(%s) \n",getprocessname(dp->d_name),dp->d_name);
	}
	if (closedir(dirf) == -1)
            errMsg("closedir");

    }

    if (errno != 0)
        errExit("readdir");

    if (closedir(dirp) == -1)
        errMsg("closedir");


    return 0;
}


char *
getprocessname(char *pid)
{
    FILE *file;
    char line[256];
    char *name = (char*) malloc(256);
    char * filename = strdup("/proc/");
    strcat(filename, pid);
    strcat(filename, "/status");
    file = fopen(filename,"r");
    while((fgets(line, sizeof line, file) !=NULL))
    {
        if(strncmp(line, "Name:", 5) ==0)
	{
	    sscanf(line,"%*5s %s", name);
	}
    }
    return name;	    
}






