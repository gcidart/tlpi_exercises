/*
 *Recode the sequence-number client-server application of Section 44.8 to use POSIX message queues.
 * 
 * $ ./pmsg/mq_seqnum_client 1
 * Received request from client  : 28225
 * Client Pid 28225 received SeqNum: 0
 * $ ./pmsg/mq_seqnum_client 2
 * Received request from client  : 28226
 * Client Pid 28226 received SeqNum: 1
 * $ ./pmsg/mq_seqnum_client 4
 * Received request from client  : 28227
 * Client Pid 28227 received SeqNum: 3
 * $ ./pmsg/mq_seqnum_client 3
 * Received request from client  : 28228
 * Client Pid 28228 received SeqNum: 7
 * 
 */

#include <signal.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"


#define SERVER_MQ "/sv_mq"
                                /* Well-known name for server's Message Queue */
#define CLIENT_MQ_TEMPLATE "/cl_mq.%ld"
                                /* Template for building client Message Queue name */
#define CLIENT_MQ_NAME_LEN (sizeof(CLIENT_MQ_TEMPLATE) + 20)
                                /* Space required for client Message Queue pathname
                                  (+20 as a generous allowance for the PID) */

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
    struct request req;
    int mqd, cqd ;
    unsigned int prio;
    char clientMQ[CLIENT_MQ_NAME_LEN];
    void *cbuffer;
    struct mq_attr cattr;


    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [seq-len]\n", argv[0]);

    /* Create our Message Queue (before sending request, to avoid a race) */

    snprintf(clientMQ, CLIENT_MQ_NAME_LEN, CLIENT_MQ_TEMPLATE,
                (long) getpid());
    cqd = mq_open(clientMQ, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, NULL);
        if (cqd == (mqd_t) -1)
            errExit("client mq_open");

    if (mq_getattr(cqd, &cattr) == -1)
        errExit("mq_getattr");

    cbuffer = malloc(cattr.mq_msgsize);
    if (cbuffer == NULL)
        errExit("malloc");


    /* Construct request message, open server Message Queue, and send message */

    req.pid = getpid();
    req.seqLen = (argc > 1) ? atoi(argv[1]) : 1;

    mqd = mq_open(SERVER_MQ, O_RDWR );
    if (mqd == (mqd_t) -1)
        errExit("Server mq_open");
    

    if (mq_send(mqd,(char *) &req, sizeof(struct request), 0) == -1)
        fatal("Can't write to server");

    /* Read our Message Queue display response */


    if (mq_receive(cqd, cbuffer, cattr.mq_msgsize, &prio)
                != sizeof(struct response)) {
            fprintf(stderr, "Error reading request; discarding\n");
            fatal("Can't read response from server");
        }

    printf("Client Pid %ld received SeqNum: %d\n", (long) getpid(), ((struct response *) cbuffer)->seqNum);
    exit(EXIT_SUCCESS);
}
