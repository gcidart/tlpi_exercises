/*
 *Rewrite the file-server application of Section 46.8 to use POSIX message queues instead of System V message queues
 * 
 * 
 * $ wc -c test/test.txt 
 * 66832356 test/test.txt
 * $ ./pmsg/mq_file_server &
 * [1] 30656
 * $ ./pmsg/mq_file_client test/test.txt 
 * Received request from client 30657
 * Received 66832356 bytes (8168 messages)
 * 
 */



#include <sys/types.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <mqueue.h>
#include <sys/wait.h>
#include "tlpi_hdr.h"



#define SERVER_MQ "/sv_mq"
                                /* Well-known name for server's Message Queue */
#define CLIENT_MQ_TEMPLATE "/cl_mq.%ld"
                                /* Template for building client Message Queue name */
#define CLIENT_MQ_NAME_LEN (sizeof(CLIENT_MQ_TEMPLATE) + 20)
                                /* Space required for client Message Queue pathname
                                  (+20 as a generous allowance for the PID) */



struct requestMsg {                     /* Requests (client to server) */
    long mtype;                         /* Unused */
    int  clientId;                      /* ID of client's message queue */
    char pathname[PATH_MAX];            /* File to be returned */
};

#define RESP_MSG_SIZE 8192

struct responseMsg {                    /* Responses (server to client) */
    long mtype;                         /* One of RESP_MT_* values below */
    char data[RESP_MSG_SIZE];           /* File content / response message */
};

/* Types for response messages sent from server to client */

#define RESP_MT_FAILURE 1               /* File couldn't be opened */
#define RESP_MT_DATA    2               /* Message contains file data */
#define RESP_MT_END     3               /* File data complete */



static void             /* SIGCHLD handler */
grimReaper(int sig)
{
    int savedErrno;

    savedErrno = errno;                 /* waitpid() might change 'errno' */
    while (waitpid(-1, NULL, WNOHANG) > 0)
        continue;
    errno = savedErrno;
}

static void             /* Executed in child process: serve a single client */
serveRequest(const void *buffer)
{
    int fd;
    ssize_t numRead;
    struct responseMsg resp;
    struct requestMsg * req;
    char clientMQ[CLIENT_MQ_NAME_LEN];
    mqd_t cqd;

    req = (struct requestMsg *) buffer;

    snprintf(clientMQ, CLIENT_MQ_NAME_LEN, CLIENT_MQ_TEMPLATE,
                (long) (req->clientId));
    printf("Received request from client %ld\n", (long ) req->clientId);

    cqd = mq_open(clientMQ, O_RDWR);
    if (cqd == ((mqd_t) -1))
        errExit("Client mq_open");


    fd = open(req->pathname, O_RDONLY);
    if (fd == -1) {                     /* Open failed: send error text */
        resp.mtype = RESP_MT_FAILURE;
        snprintf(resp.data, sizeof(resp.data), "%s", "Couldn't open");
        mq_send(cqd,(char *)  &resp, sizeof(resp.data)+sizeof(resp.mtype), 0);
        exit(EXIT_FAILURE);             /* and terminate */
    }

    /* Transmit file contents in messages with type RESP_MT_DATA. We don't
       diagnose read()  errors since we can't notify client. */

    resp.mtype = RESP_MT_DATA;
    while ((numRead = read(fd, resp.data, RESP_MSG_SIZE-((int)sizeof(resp.mtype)))) > 0)
    {
        numRead+=sizeof(resp.mtype);
        if (mq_send(cqd,(char *)  &resp,  numRead, 0) == -1)
            break;
    }

    /* Send a message of type RESP_MT_END to signify end-of-file */

    resp.mtype = RESP_MT_END;
    mq_send(cqd,(char *) &resp, RESP_MSG_SIZE, 0);         /* Zero-length mtext */
}

int
main(int argc, char *argv[])
{
    pid_t pid;
    ssize_t msgLen;
    struct sigaction sa;
    unsigned int prio;
    mqd_t mqd;
    void *sbuffer;
    struct mq_attr sattr ;

    /* Create server message queue */

    mqd = mq_open(SERVER_MQ, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR, NULL);
    if (mqd == (mqd_t) -1)
        errExit("server mq_open"); 

    if (mq_getattr(mqd, &sattr) == -1)
        errExit("mq_getattr");

    sbuffer = malloc(sattr.mq_msgsize);
    if (sbuffer == NULL)
        errExit("malloc");

    /* Establish SIGCHLD handler to reap terminated children */

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = grimReaper;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        errExit("sigaction");

    /* Read requests, handle each in a separate child process */

    for (;;) {
        msgLen = mq_receive(mqd, sbuffer, sattr.mq_msgsize, &prio);
        if (msgLen == -1) {
            if (errno == EINTR)         /* Interrupted by SIGCHLD handler? */
                continue;               /* ... then restart msgrcv() */
            errMsg("msgrcv");           /* Some other error */
            break;                      /* ... so terminate loop */
        }

        pid = fork();                   /* Create child process */
        if (pid == -1) {
            errMsg("fork");
            break;
        }

        if (pid == 0) {                 /* Child handles request */
            serveRequest(sbuffer);
            _exit(EXIT_SUCCESS);
        }

        /* Parent loops to receive next client request */
    }

    exit(EXIT_SUCCESS);
}
