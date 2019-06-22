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
    pid_t  clientId;                    /* ID of client's message queue */
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

int
main(int argc, char *argv[])
{
    struct requestMsg req;
    struct responseMsg resp;
    int  numMsgs;
    ssize_t msgLen, totBytes;
    unsigned int prio;
    mqd_t mqd ,cqd;
    char clientMQ[CLIENT_MQ_NAME_LEN];



    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname\n", argv[0]);

    if (strlen(argv[1]) > sizeof(req.pathname) - 1)
        cmdLineErr("pathname too long (max: %ld bytes)\n",
                (long) sizeof(req.pathname) - 1);

    /* Get server's queue identifier; create queue for response */


    snprintf(clientMQ, CLIENT_MQ_NAME_LEN, CLIENT_MQ_TEMPLATE,
                (long) getpid());
    cqd = mq_open(clientMQ, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR, NULL);
    if (cqd == (mqd_t) -1)
            errExit("Client mq_open");

    mqd = mq_open(SERVER_MQ, O_RDWR);
    if (mqd == (mqd_t) -1)
        errExit("server mq_open"); 




    /* Send message asking for file named in argv[1] */

    req.mtype = 1;                      /* Any type will do */
    req.clientId =  getpid();
    strncpy(req.pathname, argv[1], sizeof(req.pathname) - 1);
    req.pathname[sizeof(req.pathname) - 1] = '\0';
                                        /* Ensure string is terminated */
    if(mq_send(mqd,(char *) &req, sizeof(struct requestMsg), 0)==-1)
        errExit("mq_send");

    /* Get first response, which may be failure notification */

    msgLen = mq_receive(cqd, (char *) &resp, RESP_MSG_SIZE, &prio);
    if (msgLen == -1)
        errExit("mq_receive");

    if (resp.mtype == RESP_MT_FAILURE) {
        printf("%s\n", resp.data);      /* Display msg from server */
        exit(EXIT_FAILURE);
    }
    
    /* File was opened successfully by server; process messages
       (including the one already received) containing file data */

    totBytes = 0;
    for (numMsgs = 1; resp.mtype == RESP_MT_DATA; numMsgs++) {
        totBytes += msgLen -  sizeof(resp.mtype);
        msgLen = mq_receive(cqd,(char *) &resp, RESP_MSG_SIZE, &prio);
        if (msgLen == -1)
            errExit("mq_receive");

    }

    printf("Received %ld bytes (%d messages)\n", (long) totBytes, numMsgs);

    exit(EXIT_SUCCESS);
}
