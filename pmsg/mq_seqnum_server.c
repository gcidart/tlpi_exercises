/*
 *Recode the sequence-number client-server application of Section 44.8 to use POSIX message queues.
 *
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
    unsigned int prio;
    mqd_t mqd ,cqd;
    char clientMQ[CLIENT_MQ_NAME_LEN];
    struct response resp;
    void *sbuffer;
    struct mq_attr sattr ;

    /* Create well-known Message Queue, and open it for reading */

    mqd = mq_open(SERVER_MQ, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR, NULL);
    if (mqd == (mqd_t) -1)
        errExit("server mq_open"); 

    if (mq_getattr(mqd, &sattr) == -1)
        errExit("mq_getattr");

    sbuffer = malloc(sattr.mq_msgsize);
    if (sbuffer == NULL)
        errExit("malloc");



    for (;;) {                          /* Read requests and send responses */
        if (mq_receive(mqd, sbuffer, sattr.mq_msgsize, &prio)
                == -1) {
            errExit("mq_receive");
            fprintf(stderr, "Error reading request; discarding\n");
            //continue;                   /* Either partial read or error */
        }
        printf("Received request from client  : %ld\n", (long) ((struct request *)sbuffer)->pid);

        /* Open client Message Queue (previously created by client) */

        snprintf(clientMQ, CLIENT_MQ_NAME_LEN, CLIENT_MQ_TEMPLATE,
                (long) ((struct request *)sbuffer)->pid);
        cqd = mq_open(clientMQ, O_RDWR);
        if (cqd == (mqd_t) -1)
            errExit("Client mq_open");


        if (mq_send(cqd,(char *)  &resp, sizeof(struct response), 0) == -1)
            errExit("mq_send");
        if (mq_close(cqd) == -1)
            errMsg("close");

    }
}
