/* t_tee.c
   Implement tee using I/O system calls. 
   By default, tee overwrites any existing file with the given name. 
   Implement the -a command-line option (tee -a file), which causes tee to append text to the end of a file if it already exists.
*/
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#ifndef BUF_SIZE        /* Allow "cc -D" to override definition */
#define BUF_SIZE 1024
#endif

int
main(int argc, char *argv[])
{
    int outputFd, openFlags, opt;
    mode_t filePerms;
    ssize_t numRead;
    char buf[BUF_SIZE];
    char *fileName=NULL;

    if (argc == 1 || strcmp(argv[1], "--help") == 0)
        usageErr("%s [-a] file\n", argv[0]);
    /* Open output file */

    
    while ((opt = getopt(argc, argv, "a:")) != -1) {
        switch (opt) {
        case 'a': openFlags =  O_CREAT | O_WRONLY | O_APPEND ;
	  	  fileName = optarg;
		  break;
        default:  fatal("Unexpected case in option switch()");
        }
    }
    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH;      /* rw-rw-rw- */
    if(fileName==NULL)
    {
    	fileName = argv[1];
    	openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    }
    outputFd = open(fileName, openFlags, filePerms);
    if (outputFd == -1)
        errExit("opening file %s", fileName);

    /* Transfer data until we encounter end of input or an error */

    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0){
        if (write(outputFd, buf, numRead) != numRead)
            fatal("couldn't write whole buffer");
        if (write(STDOUT_FILENO, buf, numRead) != numRead)
            fatal("couldn't write whole buffer on STDOUT");
    }
    if (numRead == -1)
        errExit("read");

    if (close(outputFd) == -1)
        errExit("close output");

    exit(EXIT_SUCCESS);
}
