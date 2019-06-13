/*
 *The server in Listing 44-7 (fifo_seqnum_server.c) always starts assigning sequence numbers from 0 each time it is started.
 *Modify the program to use a backup file that is updated each time a sequence number is assigned.
 *(The open() O_SYNC flag, described in Section 4.3.1, may be useful.)
 *At startup, the program should check for the existence of this file, and if it is present, use the value it contains to initialize the sequence number.
 *If the backup file can’t be found on startup, the program should create a new file and start assigning sequence numbers beginning at 0.
 *(An alternative to this technique would be to use memory-mapped files, described in Chapter 49.) 
 * 
 * 
 * $ ./pipes/fifo_seqnum_server_file_startup &
 * [1] 4035
 * $ ./pipes/fifo_seqnum_client 2
 * 0
 * $ ./pipes/fifo_seqnum_client 3
 * 2
 * $ ./pipes/fifo_seqnum_client 5
 * 5
 * $ ps | grep fifo
 *  4035 pts/0    00:00:00 fifo_seqnum_ser
 * $ kill 4035
 * $ ./pipes/fifo_seqnum_server_file_startup &
 * [2] 4042
 * [1]   Terminated              ./pipes/fifo_seqnum_server_file_startup
 * $ ./pipes/fifo_seqnum_client 5
 * 10
 * 
 * 
 * 
 *http://man7.org/tlpi/code/online/dist/pipes/fifo_seqnum_client.c.html
 * 
*/

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "tlpi_hdr.h"

#define SERVER_FIFO "/tmp/seqnum_sv"
                                /* Well-known name for server's FIFO */
#define CLIENT_FIFO_TEMPLATE "/tmp/seqnum_cl.%ld"
                                /* Template for building client FIFO name */
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO_TEMPLATE) + 20)
                                /* Space required for client FIFO pathname
                                  (+20 as a generous allowance for the PID) */

#define SERVER_FILE "/tmp/backup"

struct request {                /* Request (client --> server) */
    pid_t pid;                  /* PID of client */
    int seqLen;                 /* Length of desired sequence */
};

struct response {               /* Response (server --> client) */
    int seqNum;                 /* Start of sequence */
};


int
main(int argc, char *argv[])
{
    int serverFd, dummyFd, clientFd, fd;
    char clientFifo[CLIENT_FIFO_NAME_LEN];
    struct request req;
    struct response resp;
    struct stat st;
    int seqNum = 0;                     /* This is our "service" */

    /* Create well-known FIFO, and open it for reading */

    umask(0);                           /* So we get the permissions we want */
    fd = open(SERVER_FILE, O_RDWR| O_SYNC| O_CREAT, S_IRUSR|S_IWUSR);
    if (fd==-1)
        errExit("open");
    stat(SERVER_FILE, &st);
    if(st.st_size>0)
        if(read(fd, &seqNum, sizeof(int))==-1)
            errExit("read");
    if (mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1
            && errno != EEXIST)
        errExit("mkfifo %s", SERVER_FIFO);
    serverFd = open(SERVER_FIFO, O_RDONLY);
    if (serverFd == -1)
        errExit("open %s", SERVER_FIFO);

    /* Open an extra write descriptor, so that we never see EOF */

    dummyFd = open(SERVER_FIFO, O_WRONLY);
    if (dummyFd == -1)
        errExit("open %s", SERVER_FIFO);

    /* Let's find out about broken client pipe via failed write() */

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)    errExit("signal");

    for (;;) {                          /* Read requests and send responses */
        if (read(serverFd, &req, sizeof(struct request))
                != sizeof(struct request)) {
            fprintf(stderr, "Error reading request; discarding\n");
            continue;                   /* Either partial read or error */
        }

        /* Open client FIFO (previously created by client) */

        snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE,
                (long) req.pid);
        clientFd = open(clientFifo, O_WRONLY);
        if (clientFd == -1) {           /* Open failed, give up on client */
            errMsg("open %s", clientFifo);
            continue;
        }

        /* Send response and close FIFO */

        resp.seqNum = seqNum;
        if (write(clientFd, &resp, sizeof(struct response))
                != sizeof(struct response))
            fprintf(stderr, "Error writing to FIFO %s\n", clientFifo);
        if (close(clientFd) == -1)
            errMsg("close");

        seqNum += req.seqLen;           /* Update our sequence number */
        lseek(fd,0, SEEK_SET);
        if(write(fd, &seqNum, sizeof(int))==-1)
            errExit("write");
    }
}
