/*
 * Write a program that uses stat() and chmod() to perform the equivalent of chmod a+rX.
 *
 *
 * $ ls -lrd dir file prog
 * -r-x------ 1 X X    0 May 22 19:12 prog
 * -r-------- 1 X X    0 May 22 19:12 file
 * dr-------- 2 X X 4096 May 22 19:11 dir
 * $ ../files/chmod_arx prog
 * $ ../files/chmod_arx file
 * $ ../files/chmod_arx dir
 * $ ls -lrd dir file prog
 * -r-xr-xr-x 1 X X    0 May 22 19:12 prog
 * -r--r--r-- 1 X X    0 May 22 19:12 file
 * dr-xr-xr-x 2 X X 4096 May 22 19:11 dir
 */


#include <sys/sysmacros.h>
#include <sys/stat.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct stat sb;

    if ((argc != 2 && strcmp(argv[1], "--help") == 0))
        usageErr("%s  file\n");

    if (stat(argv[1], &sb) == -1)
        errExit("stat");
    
    if (chmod(argv[1], sb.st_mode| S_IRUSR | S_IRGRP | S_IROTH) == -1)
        errExit("chmod");		    
    if (stat(argv[1], &sb) == -1)
        errExit("stat");
    
    if((sb.st_mode & S_IFMT) == S_IFREG)
    {
	if(sb.st_mode & S_IXUSR)
            if (chmod(argv[1], sb.st_mode| S_IXGRP | S_IXOTH) == -1)
                errExit("chmod");
    }		    
    if((sb.st_mode & S_IFMT) == S_IFDIR)
    {
        if (chmod(argv[1], sb.st_mode| S_IXUSR| S_IXGRP | S_IXOTH) == -1)
            errExit("chmod");
    }		    
	



    exit(EXIT_SUCCESS);
}

