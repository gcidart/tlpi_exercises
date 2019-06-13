/*
 *Add code to the server in Listing 44-7 (fifo_seqnum_server.c) so that if the program receives the SIGINT or SIGTERM signals, it removes the server FIFO and terminates. 
 * 
 * $ ./pipes/fifo_seqnum_server_signal &
 * [1] 4357
 * $ ./pipes/fifo_seqnum_client 1
 * 0
 * $ ./pipes/fifo_seqnum_client 2
 * 1
 * $ kill -SIGINT 4357
 * $ 2 Interrupt
 * 
 * [1]+  Killed                  ./pipes/fifo_seqnum_server_signal
 * $ ./pipes/fifo_seqnum_server_signal &
 * [1] 4360
 * $ ./pipes/fifo_seqnum_client 3
 * 3
 * $ ./pipes/fifo_seqnum_client 4
 * 6
 * $ kill -SIGTERM 4360
 * $ 15 Terminated
 * 
 * [1]+  Killed                  ./pipes/fifo_seqnum_server_signal
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


static void
handler(int sig)
{
	printf("%d %s\n", sig, strsignal(sig));
    unlink(SERVER_FIFO);    
    raise(SIGKILL);
}


int
main(int argc, char *argv[])
{
    int serverFd, dummyFd, clientFd, fd;
    char clientFifo[CLIENT_FIFO_NAME_LEN];
    struct request req;
    struct response resp;
    struct stat st;
    struct sigaction sa;
    int seqNum = 0;                     /* This is our "service" */
    sigemptyset(&sa.sa_mask); 
    sa.sa_handler = &handler;
    sa.sa_flags = SA_RESETHAND;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);


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
