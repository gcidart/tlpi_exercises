/*
 * Implement a function that performs the equivalent of getcwd(). 
 * A useful tip for solving this problem is that you can find the name of the current working directory by using opendir() and readdir()
 * to walk through each of the entries in the parent directory (..) to find an entry with the same i-node and device number as the current working directory 
 * (i.e., respectively, the st_ino and st_dev fields in the stat structure returned by stat() and lstat()). 
 * Thus, it is possible to construct the directory path by walking up the directory tree (chdir(“..”)) one step at a time and performing such scans. 
 * The walk can be finished when the parent directory is the same as the current working directory (recall that /.. is the same as /).
 * The caller should be left in the same directory in which it started, regardless of whether your getcwd() function succeeds or fails (open() plus fchdir() are handy for this purpose). *
 */

#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

static char*             
t_getcwd(void)
{
    DIR *dirp;
    struct dirent *dp;
    int fd, i=0;
    struct stat statbuf1, statbuf2;
    ino_t inode1;
    char *temp = malloc(PATH_MAX);
    char *res = malloc(PATH_MAX);
    char **arr = malloc(PATH_MAX * sizeof(char *));

    fd = open(".", O_RDONLY);     /* Remember where we are */
    
    
    /* Each loop to go up one directory */
    
    for (;;) {
	if(lstat(".", &statbuf1)==-1)
	    errExit("lstat");
	inode1 = statbuf1.st_ino;
	if((statbuf2.st_ino==statbuf1.st_ino) && (statbuf2.st_dev==statbuf1.st_dev))
	    break;              /*If reached / */

        errno = 0;              /* To distinguish error from end-of-directory */
	dirp = opendir("../");
    	if (dirp == NULL) {
            errMsg("opendir failed");
	    fchdir(fd);                   /* Return to original directory */
            close(fd);
            return NULL;
        }
	/* Each loop to go through all directory entries of the parent directory*/
	for(;;){


            dp = readdir(dirp);
            if (dp == NULL)
                break;
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
                continue;               /* Skip . and .. */
	    if(dp->d_ino==inode1)
	    {
		strcat(temp, dp->d_name);
		arr[i++] = temp;
		temp = malloc(PATH_MAX);
		break;
	    }
	    
	    


        }
	if (errno != 0)
	{
	    fchdir(fd);
	    close(fd);
            errExit("readdir");
	}
	if (closedir(dirp) == -1)
	{
            fchdir(fd);                   /* Return to original directory */
            close(fd);
	    errMsg("closedir");
	}


	statbuf2=statbuf1;
	chdir("../");
    }
    strcpy(res,"/");
    for(int j = i-1; j>=0; j--)
    {
	strcat(res, arr[j]);
	free(arr[j]);
	strcat(res, "/");
    }
    
    fchdir(fd);                   /* Return to original directory */
    close(fd);
    
    return res;

}

int
main(int argc, char *argv[])
{
    printf("%s\n", t_getcwd());
    exit(EXIT_SUCCESS);
}
