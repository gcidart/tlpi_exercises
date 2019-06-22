/*
 *Replace the use of a signal handler in Listing 52-6 (mq_notify_sig.c) with the use of sigwaitinfo().
 *Upon return from sigwaitinfo(), display the values in the returned siginfo_t structure.
 *How could the program obtain the message queue descriptor in the siginfo_t structure returned by sigwaitinfo()?
 * 
 * 
 * $ ./pmsg/pmsg_create -cx /mqq
 * $ ./pmsg/mq_notify_sigwaitinfo /mqq &
 * [1] 31307
 * $ ./pmsg/pmsg_send /mqq msg-a 5
 * Received signo 10(User defined signal 1), Message Queue 0x7ffc9776a5d4 
 * Read 5 bytes
 * $ ./pmsg/pmsg_send /mqq msg-aa 10
 * Received signo 10(User defined signal 1), Message Queue 0x7ffc9776a5d4 
 * Read 6 bytes
 * $ ./pmsg/pmsg_send /mqq msg-aabcd 10
 * Received signo 10(User defined signal 1), Message Queue 0x7ffc9776a5d4 
 * Read 9 bytes
 * 
 *http://man7.org/tlpi/code/online/dist/pmsg/pmsg_create.c.html
 *http://man7.org/tlpi/code/online/dist/pmsg/pmsg_send.c.html
 *
 */
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>              /* For definition of O_NONBLOCK */
#include "tlpi_hdr.h"

#define NOTIFY_SIG SIGUSR1


/* This program does not handle the case where a message already exists on
   the queue by the time the first attempt is made to register for message
   notification. In that case, the program would never receive a notification.
   See mq_notify_via_signal.c for an example of how to deal with that case. */

int
main(int argc, char *argv[])
{
    struct sigevent sev;
    mqd_t mqd, *pmqd;
    struct mq_attr attr;
    void *buffer;
    ssize_t numRead;
    sigset_t blockMask ;
    siginfo_t st;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s mq-name\n", argv[0]);

    mqd = mq_open(argv[1], O_RDONLY | O_NONBLOCK);
    if (mqd == (mqd_t) -1)
        errExit("mq_open");

    /* Determine mq_msgsize for message queue, and allocate an input buffer
       of that size */

    if (mq_getattr(mqd, &attr) == -1)
        errExit("mq_getattr");

    buffer = malloc(attr.mq_msgsize);
    if (buffer == NULL)
        errExit("malloc");

    /* Block the notification signal and establish a handler for it */

    sigemptyset(&blockMask);
    sigaddset(&blockMask, NOTIFY_SIG);
    if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1)
        errExit("sigprocmask");


    /* Register for message notification via a signal */

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = NOTIFY_SIG;
    sev.sigev_value.sival_ptr = &mqd;  /* Send the message queue descriptor alongwith the signal */
    if (mq_notify(mqd, &sev) == -1)
        errExit("mq_notify");


    for (;;) {
        if(sigwaitinfo(&blockMask, &st)==-1)         /* Wait for notification signal */
            errExit("sigwaitinfo");

        /* Reregister for message notification */

        if (mq_notify(mqd, &sev) == -1)
            errExit("mq_notify");
        pmqd =(mqd_t *) st.si_value.sival_ptr;

        printf("Received signo %d(%s), Message Queue %p \n", st.si_signo, strsignal(st.si_signo), st.si_value.sival_ptr);

        while ((numRead = mq_receive(*pmqd, buffer, attr.mq_msgsize, NULL)) >= 0)
            printf("Read %ld bytes\n", (long) numRead);

        if (errno != EAGAIN)            /* Unexpected error */
            errExit("mq_receive");
    }
}
