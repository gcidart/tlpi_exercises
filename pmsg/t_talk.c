/*
 Write a simple chat program (similar to talk(1), but without the curses interface) using POSIX messages queues.
 * 
 * $ ./pmsg/t_talk user
 * Hello user
 * Let's Talk
 * ^C
 * 
 * $ ./pmsg/t_talk -s user
 * Hello user
 * Let's Talk
 * ^\Quit (core dumped)
 * 
 * 
 */


#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "tlpi_hdr.h"

#define MQ_TEMPLATE "/chat_mq%s"
                                /* Template for building client Message Queue name */
#define MQ_NAME_LEN (sizeof(MQ_TEMPLATE) + 20)
                                /* Space required for client Message Queue pathname
                                  (+20 as a generous allowance for the PID) */

#define USERNAME_LEN 20

static void
usageError(const char *progName)
{
    fprintf(stderr, "Usage: %s [-s] user-name \n", progName);
    fprintf(stderr, "    -s          Sign in as user-name\n");
    exit(EXIT_FAILURE);
}
int
main(int argc, char *argv[])
{
    int flags, opt;
    mode_t perms;
    mqd_t mqd;
    char user[USERNAME_LEN];
    char qname[MQ_NAME_LEN];
    int rx;
    struct mq_attr attr;
    void *buffer;
    sigset_t blockMask;
    struct sigevent sev;
    siginfo_t si;
    ssize_t numRead;

    flags = O_RDWR;
    rx = 0;

    /* Parse command-line options */

    while ((opt = getopt(argc, argv, "s")) != -1) {
        switch (opt) {
        case 's':
            flags |= O_CREAT;
            rx = 1;
            break;

        default:
            usageError(argv[0]);
        }
    }
    if (optind >= argc)
        usageError(argv[0]);
    strncpy(user, argv[optind], USERNAME_LEN-1);
    user[USERNAME_LEN-1] = '\0';
    snprintf(qname, MQ_NAME_LEN, MQ_TEMPLATE, user);
    perms =  S_IRWXU | S_IRWXG | S_IRWXO;

    mqd = mq_open(qname, flags, perms, NULL);
    if (mqd == (mqd_t) -1)
        errExit("mq_open");

    if (mq_getattr(mqd, &attr) == -1)
        errExit("mq_getattr");

    buffer = malloc(attr.mq_msgsize);
    if (buffer == NULL)
        errExit("malloc");

    if(rx ==1)
    {
        /* Block the signal that we'll accept using sigwaitinfo() */

        sigemptyset(&blockMask);
        sigaddset(&blockMask, SIGUSR1);
        if (sigprocmask(SIG_BLOCK, &blockMask, NULL) == -1)
            errExit("sigprocmask");

        /* Set up message notification using the signal SIGUSR1 */

        sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_signo = SIGUSR1;

        if (mq_notify(mqd, &sev) == -1)
            errExit("mq_notify");

        for(;;)
        {
            if (sigwaitinfo(&blockMask, &si) == -1)
                errExit("sigwaitinfo");

            /* Reestablish message notification */

            if (mq_notify(mqd, &sev) == -1)
                errExit("mq_notify");

            while (((numRead = mq_receive(mqd, buffer, attr.mq_msgsize, NULL)) >= 0)|| (errno =EINTR))
            {
                errno = 0 ;
                write(STDOUT_FILENO, buffer, numRead);
            }
        }
    }
    else
    {
        while((numRead = read(STDIN_FILENO, buffer, sizeof(char)))>0)
        {
            if(mq_send(mqd, buffer, attr.mq_msgsize, 0)==-1)
                errExit("mq_send");
        }
    }
    exit(EXIT_SUCCESS);
}
