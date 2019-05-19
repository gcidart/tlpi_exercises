/*
 Write a program that draws a tree showing the hierarchical parent-child relationships of all processes on the system, going all the way back to init.
 For each process, the program should display the process ID and the command being executed.
 The output of the program should be similar to that produced by pstree(1), although it does need not to be as sophisticated. 
 The parent of each process on the system can be found by inspecting the PPid: line of all of the /proc/PID/status files on the system. 
 Be careful to handle the possibility that a process’s parent (and thus its /proc/PID directory) disappears during the scan of all /proc/PID directories.
 
 $ ./t_pstree 
 systemd(1)
 -----firefox(2781)
 ----------Web(3199)
 ----------Web(3157)
 ----------WebExtensions(2997)
 ----------Web(2873)
 .
 .
 
 
 
 */

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include "tlpi_hdr.h"

struct child
{
    int val;
    struct child * next;
};

char *
getprocessname(char *pid);
char *
getprocessppid(char *pid);
void
dfs(struct child **arr, int u, int depth);



int
main(int argc, char *argv[])
{
    struct child *arr[10000];
    int pid, ppid;
    DIR *dirp;
    struct dirent *dp;
    char *endptr;
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
	pid = (int) strtol(dp->d_name, &endptr, 10);      
    	if (*endptr != '\0')                /* only allow  numeric string */
            continue;
	ppid = (int) strtol(getprocessppid(dp->d_name), &endptr, 10);
	struct child *temp = arr[ppid];
	arr[ppid] = malloc(sizeof(struct child));
	arr[ppid]->val = pid;
	arr[ppid]->next = temp;
    }

    if (errno != 0)
        errExit("readdir");

    if (closedir(dirp) == -1)
        errMsg("closedir");
    dfs(arr, 1, 0);


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
    fclose(file);
    return name;	    
}

char *
getprocessppid(char *pid)
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
        if(strncmp(line, "PPid:", 5) ==0)
	{
	    sscanf(line,"%*5s %s", name);
	}
    }
    return name;	    
}

void
dfs(struct child **arr, int u, int depth)
{
    char *ph = (char*) malloc(256);
    char *process = (char*)malloc(20);
    char *processname ;
    sprintf(process, "%d", u);
    processname = getprocessname(process);
    strcat(processname,"(");
    strcat(processname,process);
    strcat(processname,")");
    strcpy(ph,"");
    for(int i=0; i< depth; i++)
        strcat(ph,"-----");
    printf("%s%s\n", ph,processname);
    free(ph);
    free(process);
    free(processname);
    struct child *it = arr[u];
    while(it!=NULL)
    {
        dfs(arr, it->val, depth+1);
	it = it->next;
    }
}

