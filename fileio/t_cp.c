/* t_cp.c
 * Write a program like cp that, when used to copy a regular file that contains holes (sequences of null bytes),
 * also creates corresponding holes in the target file.
 * du -h <file> gives the size of the file on disk; ls -l gives apparent size
 * cp --sparse=never SOURCE DEST inhibits creation of sparse files
 * t_cp implements the behavior of "cp --sparse=always"
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#ifndef BUF_SIZE        /* Allow "cc -D" to override definition */
#define BUF_SIZE 1024
#endif

int
main(int argc, char *argv[])
{
    int inputFd, outputFd, openFlags;
    mode_t filePerms;
    ssize_t numRead, dataBlock;
    char buf[BUF_SIZE];

    if (argc == 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s  SOURCE DEST\n", argv[0]);
    /* Open input and output files */

    inputFd = open(argv[1], O_RDONLY);
    if (inputFd == -1)
        errExit("opening file %s", argv[1]);

    openFlags = O_CREAT | O_WRONLY | O_TRUNC;
    filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH;      /* rw-rw-rw- */
    outputFd = open(argv[2], openFlags, filePerms);
    if (outputFd == -1)
        errExit("opening file %s", argv[2]);

    off_t dataOffset = lseek(inputFd, 0, SEEK_DATA);
    /*Find first hole instance after first data in sparse file */
    off_t holeOffset = lseek(inputFd, dataOffset, SEEK_HOLE);
    /*Set back the offset to data location*/
    lseek(inputFd, dataOffset, SEEK_SET);

    while(dataOffset != -1)
    {
	    /*Length of block of data at current offset */
	    dataBlock = holeOffset - dataOffset;
	    /*Move the offset for output file to match the SEEK_DATA offset of input file*/
	    lseek(outputFd, dataOffset, SEEK_SET);
	    while(dataBlock > 0)
	    {
		     
		    if(dataBlock > BUF_SIZE)
		    {
			    numRead = read(inputFd, buf, BUF_SIZE);
    			    if (numRead == -1)
        			errExit("read");
			    if (write(outputFd, buf, numRead) != numRead)
            			fatal("couldn't write whole buffer");
		    }
		    else
		    {
			    numRead = read(inputFd, buf, dataBlock);
    			    if (numRead == -1)
        			errExit("read");
			    if (write(outputFd, buf, numRead) != numRead)
            			fatal("couldn't write whole buffer");
		    }
		    dataBlock -= BUF_SIZE;
	    }
	    dataOffset = lseek(inputFd, holeOffset, SEEK_DATA);
	    holeOffset = lseek(inputFd, dataOffset, SEEK_HOLE);
	    /*Set back the offset to data location*/
	    lseek(inputFd, dataOffset, SEEK_SET);
    }

    if (close(inputFd) == -1)
        errExit("close input");

    if (close(outputFd) == -1)
        errExit("close output");

    exit(EXIT_SUCCESS);
}
