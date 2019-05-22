/* 
 * Write a program that measures the time required to create and then remove a large number of 1-byte files from a single directory. 
 * The program should create files with names of the form xNNNNNN, where NNNNNN is replaced by a random six-digit number. 
 * The files should be created in the random order in which their names are generated, and then deleted in increasing numerical order
 * (i.e., an order that is different from that in which they were created).
 * The number of files (NF) and the directory in which they are to be created should be specifiable on the command line.
 * Measure the times required for different values of NF (e.g., in the range from 1000 to 20,000) and for different file systems (e.g., ext2, ext3, and XFS). 
 * What patterns do you observe on each file system as NF increases? 
 * How do the various file systems compare? 
 * Do the results change if the files are created in increasing numerical order (x000000, x000001, x0000002, and so on) and then deleted in the same order? 
 * If so, what do you think the reason(s) might be? Again, do the results vary across file-system types?
 *
 *
 * Test run on ext4 Filesystem: 
 *	$ time ./filesys/file_create_delete 10000 ./test/file_cr_de/ 
 *
 *	real	0m0.813s
 *	user	0m0.024s
 *	sys	0m0.246s
 *
 *	$ time ./filesys/file_create_delete 10000 ./test/file_cr_de/ s
 *
 *	real	0m0.823s
 *	user	0m0.035s
 *	sys	0m0.241s
 * 
 *
 *
 *	$ time ./filesys/file_create_delete 100000 ./test/file_cr_de/ 
 *
 *	real	0m8.176s
 *	user	0m0.157s
 *	sys	0m2.642s
 *
 *
 *
 *	$ time ./filesys/file_create_delete 100000 ./test/file_cr_de/ s
 *
 *	real	0m8.392s
 *	user	0m0.274s
 *	sys	0m2.629s
 * 
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "tlpi_hdr.h"

#ifndef BUF_SIZE        
#define BUF_SIZE 2
#endif

int 
cmpfunc (const void * a, const void * b);

int
main(int argc, char *argv[])
{
    int Fd;
    char buf[BUF_SIZE];
    char *pathName, *fileName;
    char *endptr;
    int num_files;
    int *rand_Arr;


    if (argc <3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s num_files directory [s]\n", argv[0]);
    
    num_files = strtol(argv[1], &endptr, 10);
    
    rand_Arr = (int *) malloc(sizeof(int) * num_files);
    strcpy(buf, "z");
    for(int i =0; i<num_files; i++)
    {
	rand_Arr[i] = rand()%1000000;
	pathName = malloc(4096);
	fileName = malloc(1024);
	strcpy(pathName,argv[2]);
	sprintf(fileName,"x%06d", rand_Arr[i]);
	strcat(pathName, fileName);
	struct stat chk;
	if(stat(pathName, &chk) != 0)
	{
	    Fd = open(pathName, O_CREAT|O_WRONLY, S_IRWXU|S_IRWXG);
	    if(Fd == -1)
	        errExit("opening file %s", pathName);
	    write(Fd, buf, BUF_SIZE);
	    if (close(Fd) == -1)
                errExit("closing file %s", pathName);
	}
	else
            i--;
	free(pathName);
	free(fileName);
    }
    if(argc==4)
    {
	qsort(rand_Arr, num_files, sizeof(int), cmpfunc);
    }
    for(int i =0; i<num_files; i++)
    {
	pathName = malloc(4096);
	fileName = malloc(1024);
	strcpy(pathName,argv[2]);
	sprintf(fileName,"x%06d", rand_Arr[i]);
	strcat(pathName, fileName);
	if (unlink(pathName) == -1)
            errExit("deleting file %s", pathName);
	free(pathName);
	free(fileName);
    }

    

    
    exit(EXIT_SUCCESS);
}

int
cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}
