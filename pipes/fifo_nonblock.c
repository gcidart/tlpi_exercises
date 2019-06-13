/*
 *Write programs to verify the operation of nonblocking opens and nonblocking I/O on FIFOs (see Section 44.9).
 * 
 * $ ./pipes/fifo_nonblock 
 * errno on opening write end of FIFO: ENXIO
 * errno on reading empty FIFO : EAGAIN/EWOULDBLOCK  
 */

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "tlpi_hdr.h"
#include "ename.c.inc"      

#define TEST_FIFO "/tmp/testFIFO"



int
main(int argc, char *argv[])
{
    int readFd, writeFd;
    int seqNum = 0;                     


    umask(0);                           /* So we get the permissions we want */
    if (mkfifo(TEST_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1
            && errno != EEXIST)
        errExit("mkfifo %s", TEST_FIFO);
    writeFd = open(TEST_FIFO, O_WRONLY|O_NONBLOCK);
    if (writeFd == -1)
        printf("errno on opening write end of FIFO: %s\n", ename[errno]);
    errno = 0;


    readFd = open(TEST_FIFO, O_RDONLY|O_NONBLOCK);
    if (readFd == -1)
        errExit("open %s", TEST_FIFO);
 
    writeFd = open(TEST_FIFO, O_WRONLY|O_NONBLOCK);
    if (writeFd == -1)
        printf("errno on opening write end of FIFO: %s\n", ename[errno]);
    errno = 0;

    if(read(readFd, &seqNum, sizeof(int))==-1)
        printf("errno on reading empty FIFO : %s\n", ename[errno]);
    errno = 0;
    
    seqNum = 42;
    
    if(write(writeFd, &seqNum, sizeof(int))==-1)
        errExit("write");

    seqNum = 0;
    if(read(readFd, &seqNum, sizeof(int))==-1)
        printf("errno on reading empty FIFO : %s\n", ename[errno]);
    errno = 0;
    

}
