/*
 This exercise is designed to demonstrate why the atomicity guaranteed by opening a file with the O_APPEND flag is necessary. 
 Write a program that takes up to three command-line arguments:

 $ atomic_append <filename> <num-bytes> [x]

 This program should open the specified filename (creating it if necessary) and 
 append num-bytes bytes to the file by using write() to write a byte at a time.
 By default, the program should open the file with the O_APPEND flag, 
 but if a third command-line argument (x) is supplied, the O_APPEND flag should be omitted, 
 and instead, the program should perform and lseek(fd, 0, SEEK_END) call before each write(). 
 Run two instances of this program at the same time without the x argument to write 1 million bytes to the same file:

 $ atomic_append f1 1000000 & atomic_append f1 1000000

 Repeat the same steps, writing to a different file, but this time specifying the x argument:

 $ atomic_append f2 1000000 x & atomic_append f2 1000000 x

 List the sizes of the files f1 and f2 using ls -l and explain the difference.


 -rwx------ 1 aa aa 2000000 May 13 11:48 f1
 -rwx------ 1 aa aa 1999717 May 13 11:48 f2
 As lseek and write combination is not atomic, race condition without O_APPEND flag leads to the difference in filesize.


*/
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int fd;

    if (argc < 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s filename num-bytes [x] \n", argv[0]);

    if(argc ==4)
    {
    	fd = open(argv[1], O_WRONLY |O_CREAT , S_IRWXU );
	if (fd == -1)
        	errExit("open");
	for(int i =0; i <atol(argv[2]); i++)
	{
		lseek(fd, 0, SEEK_END);
		if (write(fd, "x", 1) == -1)
        		errExit("write");
	}
    }
    else
    {
    	fd = open(argv[1], O_WRONLY |O_CREAT |O_APPEND, S_IRWXU );
	if (fd == -1)
        	errExit("open");
	for(int i =0; i < atol(argv[2]); i++)
	{
		if (write(fd, "x", 1) == -1)
        		errExit("write");
	}
    }
    exit(EXIT_SUCCESS);
}

