/*
 Write a simple version of the chattr(1) command, which modifies file i-node flags. 
 See the chattr(1) man page for details of the chattr command-line interface. 
 (You don’t need to implement the –R, –V, and –v options.) 

$ lsattr file
------------------ file
$ sudo ../files/t_chattr +i file
$ lsattr file
----i------------- file
$ sudo ../files/t_chattr +js file
$ lsattr file
s---i-----j------- file
$ sudo ../files/t_chattr -jsi file
$ lsattr file
------------------ file
$ sudo ../files/t_chattr =acd file
$ lsattr file
-----ad-c--------- file
 
 
 */


#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int attr, attr1=0;
    int Fd;

    if ((argc != 3 && strcmp(argv[1], "--help") == 0))
        usageErr("%s {+|-|=}[aAcdDijsStTu]  file\n");

    for(int i=1; i<strlen(argv[1]); i++)
    {
	if(argv[1][i] =='A')
	    attr1 |=FS_NOATIME_FL;
        else if(argv[1][i] =='a')
	    attr1 |=FS_APPEND_FL;
        else if(argv[1][i] =='c')
	    attr1 |=FS_COMPR_FL;
        else if(argv[1][i] =='d')
	    attr1 |=FS_NODUMP_FL;
        else if(argv[1][i] =='D')
	    attr1 |=FS_DIRSYNC_FL;
        else if(argv[1][i] =='i')
	    attr1 |=FS_IMMUTABLE_FL;
        else if(argv[1][i] =='j')
	    attr1 |=FS_JOURNAL_DATA_FL;
        else if(argv[1][i] =='s')
	    attr1 |=FS_SECRM_FL;
        else if(argv[1][i] =='S')
	    attr1 |=FS_SYNC_FL;
        else if(argv[1][i] =='t')
	    attr1 |=FS_NOTAIL_FL;
        else if(argv[1][i] =='T')
	    attr1 |=FS_TOPDIR_FL;
        else if(argv[1][i] =='u')
	    attr1 |=FS_UNRM_FL;
    }
    Fd = open(argv[2], O_RDONLY);
    if(Fd==-1)
	errExit("open");
    if(argv[1][0] == '=')
    {
	if (ioctl(Fd, FS_IOC_SETFLAGS, &attr1) == -1)    /* Update flags */
            errExit("ioctl");
    }
    else
    {
	if (ioctl(Fd, FS_IOC_GETFLAGS, &attr) == -1)    /* Fetch current flags */
            errExit("ioctl");
	if(argv[1][0]=='+')
	    attr |= attr1;
	else if(argv[1][0]=='-')
	    attr = attr &(~attr1);
	if (ioctl(Fd, FS_IOC_SETFLAGS, &attr) == -1)    /* Update flags */
            errExit("ioctl");
    }


	    
	



    exit(EXIT_SUCCESS);
}

