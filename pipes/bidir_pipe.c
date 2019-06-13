/*
 *Write a program that uses two pipes to enable bidirectional communication between a parent and child process.
 *The parent process should loop reading a block of text from standard input and use one of the pipes to send the text to the child,
 * which converts it to uppercase and sends it back to the parent via the other pipe.
 *The parent reads the data coming back from the child and echoes it on standard output before continuing around the loop once more.
 *
 * $ ./pipes/bidir_pipe 
 * Testing this Program
 * TESTING THIS PROGRAM
 * Again testing
 * AGAIN TESTING
 * 
*/

#include <sys/wait.h>
#include <ctype.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 10

int
main(int argc, char *argv[])
{
    int pfd1[2], pfd2[2];                             /* Pipe file descriptors */
    char buf[BUF_SIZE];
    ssize_t numRead;


    if (pipe(pfd1) == -1)                    /* Create the pipe */
        errExit("pipe");
    if (pipe(pfd2) == -1)                    /* Create the pipe */
        errExit("pipe");

    switch (fork()) {
    case -1:
        errExit("fork");

    case 0:                     
        if (close(pfd1[1]) == -1)            /* Write end is unused */
            errExit("close - child");
        if (close(pfd2[0]) == -1)            /* Read end is unused */
            errExit("close - child");

        for (;;) {              
            numRead = read(pfd1[0], buf, BUF_SIZE);
            if (numRead == -1)
                errExit("read");
            if (numRead == 0)
                break;                      /* End-of-file */
            for(int i=0; i < strlen(buf); i++)
            {
                buf[i] = toupper(buf[i]);
            }
            if (write(pfd2[1], (buf), numRead) != numRead)
                fatal("child - partial/failed write");
        }

        if (close(pfd1[0]) == -1)
            errExit("close");
        if (close(pfd2[1]) == -1)
            errExit("close");
        _exit(EXIT_SUCCESS);

    default:            
        if (close(pfd1[0]) == -1)            /* Read end is unused */
            errExit("close - parent");
        if (close(pfd2[1]) == -1)            /* Write end is unused */
            errExit("close - parent");

        while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0)
        {
            if (write(pfd1[1], buf, numRead) != numRead)
                fatal("couldn't write whole buffer");
            if (read(pfd2[0], buf, BUF_SIZE)!= numRead)
                fatal("Couldn't read");
            if (write(STDOUT_FILENO, buf, numRead) != numRead)
                fatal("couldn't write whole buffer");
        }

        if (numRead == -1)
         errExit("read");


        if (close(pfd1[1]) == -1)            /* Child will see EOF */
            errExit("close");
        wait(NULL);                         /* Wait for child to finish */
        if (close(pfd2[0]) == -1)                        
            errExit("close");
        exit(EXIT_SUCCESS);
    }
}
