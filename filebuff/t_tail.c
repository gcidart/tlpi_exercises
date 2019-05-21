/* t_tail.c
   The command tail [ –n num ] file prints the last num lines (ten by default) of the named file. 
   Implement this command using I/O system calls (lseek(), read(), write(), and so on). 
   Keep in mind the buffering issues described in this chapter, in order to make the implementation efficient.
   
   
   Tested with BUF_SIZE = {1,10,15, 1024} on a small text file
*/

#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "tlpi_hdr.h"

#ifndef BUF_SIZE        
#define BUF_SIZE 1024
#endif

int
main(int argc, char *argv[])
{
    int inputFd,  opt;
    ssize_t numRead, toBeRead;
    char buf[BUF_SIZE];
    char *fileName=NULL;
    char *endptr;
    off_t lpos, npos;
    int num =10;


    if (argc == 1 || strcmp(argv[1], "--help") == 0)
        usageErr("%s [-n num] file\n", argv[0]);
    
    
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
        case 'n': num = (int) strtol(optarg, &endptr, 10);
		  break;
        default:  fatal("Unexpected case in option switch()");
        }
    }
    num++;
    if(argc==2)
    {
    	fileName = argv[1];
    }
    else
    {
	fileName = argv[3];
    }
    inputFd = open(fileName, O_RDONLY);
    if (inputFd == -1)
        errExit("opening file %s", fileName);
    lpos = lseek(inputFd, 0, SEEK_END);
    toBeRead = BUF_SIZE;
    if(lpos<BUF_SIZE)
    {
	toBeRead = lpos;
	lpos = lseek(inputFd,0, SEEK_SET);
    }
    else
	lpos = lseek(inputFd, 0-BUF_SIZE, SEEK_END);
    npos = 0;
    while(num!=0)
    {
        if((numRead = read(inputFd, buf, toBeRead)) > 0)
	{
	    for(int i = numRead-1; i >=0; i--)
	    {
	        if(buf[i]=='\n')
		    num--;
		npos++;
	    	if(num==0)
	            break;
	    }
	    if(lpos<BUF_SIZE)
	    {
		toBeRead = lpos;
		lpos = lseek(inputFd,0, SEEK_SET);
	    }
	    else
	    	lpos = lseek(inputFd, (0-npos-BUF_SIZE), SEEK_END);
	    
	}
	else if (numRead==-1)
	{
            errExit("read");
	}
	else 
	{
	    npos++; // To handle the case where num is less than or equal to the number of lines in the file
	    break;
	}
    }
    lseek(inputFd, 1-npos, SEEK_END);


    while ((numRead = read(inputFd, buf, BUF_SIZE)) > 0){
        if (write(STDOUT_FILENO, buf, numRead) != numRead)
            fatal("couldn't write whole buffer on STDOUT");
    }
    if (numRead == -1)
        errExit("read");

    if (close(inputFd) == -1)
        errExit("close output");

    exit(EXIT_SUCCESS);
}
