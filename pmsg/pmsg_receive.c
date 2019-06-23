/*
 *Modify the program in Listing 52-5 (pmsg_receive.c) to accept a timeout (a relative number of seconds) on the command line, and use mq_timedreceive() instead of mq_receive().
 * 
 * 
 * $ ./pmsg_create -cx /mq
 * $ ./pmsg_send  /mq msg-b 10
 * $ ./pmsg_receive  /mq 10 &
 * [1] 25722
 * $ /mq
 * Read 5 bytes; priority = 10
 * msg-b
 * 
 * [1]+  Done                    ./pmsg_receive /mq 10
 * $ ./pmsg_receive  /mq 10 &
 * [1] 25723
 * $
 * 
 * $ sleep 15; ./pmsg_send  /mq msg-d 20 &
 * ERROR [ETIMEDOUT Connection timed out] mq_receive
 * [1]+  Exit 1                  ./pmsg_receive /mq 10
 * [1] 25725
 * $ ./pmsg_receive  /mq 10 
 * Read 5 bytes; priority = 20
 * msg-d
 * [1]+  Done                    ./pmsg_send /mq msg-d 20
 * 
 *http://man7.org/tlpi/code/online/dist/pmsg/pmsg_create.c.html
 *http://man7.org/tlpi/code/online/dist/pmsg/pmsg_send.c.html
 * 
 */
#include <mqueue.h>
#include <fcntl.h>              /* For definition of O_NONBLOCK */
#include <time.h>
#include "tlpi_hdr.h"

static void
usageError(const char *progName)
{
    fprintf(stderr, "Usage: %s [-n] mq-name timeout\n", progName);
    fprintf(stderr, "    -n           Use O_NONBLOCK flag\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int flags, opt;
    mqd_t mqd;
    unsigned int prio;
    void *buffer;
    struct mq_attr attr;
    ssize_t numRead;
    struct timespec ct;

    flags = O_RDONLY;
    while ((opt = getopt(argc, argv, "n")) != -1) {
        switch (opt) {
        case 'n':   flags |= O_NONBLOCK;        break;
        default:    usageError(argv[0]);
        }
    }

    if (optind >= argc)
        usageError(argv[0]);

    if(clock_gettime(CLOCK_REALTIME, &ct)==-1)
        errExit("clk_gettime");

    ct.tv_sec+= atol(argv[optind+1]);

    

    mqd = mq_open(argv[optind], flags);
    if (mqd == (mqd_t) -1)
        errExit("mq_open");

    /* We need to know the 'mq_msgsize' attribute of the queue in
       order to determine the size of the buffer for mq_receive() */

    if (mq_getattr(mqd, &attr) == -1)
        errExit("mq_getattr");

    buffer = malloc(attr.mq_msgsize);
    if (buffer == NULL)
        errExit("malloc");

    numRead = mq_timedreceive(mqd, buffer, attr.mq_msgsize, &prio, &ct);
    if (numRead == -1)
        errExit("mq_receive");

    printf("Read %ld bytes; priority = %u\n", (long) numRead, prio);
    if (write(STDOUT_FILENO, buffer, numRead) == -1)
        errExit("write");
    write(STDOUT_FILENO, "\n", 1);

    exit(EXIT_SUCCESS);
}

