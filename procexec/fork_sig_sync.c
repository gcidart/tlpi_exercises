/*
 *
 * Suppose that in the program in Listing 24-6, the child process also needed to wait on the parent to complete some actions.
 * What changes to the program would be required in order to enforce this?
 * 
 * $ ./procexec/fork_sig_sync 
 * [12:05:05 4181] Parent about to wait for signal
 * [12:05:05 4182] Child started - doing some work
 * [12:05:07 4182] Child about to signal parent
 * [12:05:07 4182] Child about to wait for signal
 * [12:05:07 4181] Parent got signal
 * [12:05:07 4181] Parent about to signal child
 * [12:05:07 4182] Child got signal

*/

#include <signal.h>
#include <time.h>

#include "tlpi_hdr.h"

#define SYNC_SIG SIGUSR1                /* Synchronization signal */
#define BUF_SIZE 1000

static char *
currTime(const char *format)
{
    static char buf[BUF_SIZE];  /* Nonreentrant */
    time_t t;
    size_t s;
    struct tm *tm;

    t = time(NULL);
    tm = localtime(&t);
    if (tm == NULL)
        return NULL;

    s = strftime(buf, BUF_SIZE, (format != NULL) ? format : "%c", tm);

    return (s == 0) ? NULL : buf;
}

static void             /* Signal handler - does nothing but return */
handler(int sig)
{
}

int
main(int argc, char *argv[])
{
    pid_t childPid;
    sigset_t blockMask, origMask, emptyMask;
    struct sigaction sa;

    setbuf(stdout, NULL);               /* Disable buffering of stdout */

    sigemptyset(&blockMask);
    sigaddset(&blockMask, SYNC_SIG);    /* Block signal */
    if (sigprocmask(SIG_BLOCK, &blockMask, &origMask) == -1)
        errExit("sigprocmask");

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;
    if (sigaction(SYNC_SIG, &sa, NULL) == -1)
        errExit("sigaction");

    switch (childPid = fork()) {
    case -1:
        errExit("fork");

    case 0: /* Child */

        /* Child does some required action here... */

        printf("[%s %ld] Child started - doing some work\n",
                currTime("%T"), (long) getpid());
        sleep(2);               /* Simulate time spent doing some work */

        sigemptyset(&blockMask);
        sigaddset(&blockMask, SIGUSR2);    /* Block signal */
        if (sigprocmask(SIG_BLOCK, &blockMask, &origMask) == -1)
            errExit("sigprocmask");

        if (sigaction(SIGUSR2, &sa, NULL) == -1)
            errExit("sigaction");

        /* And then signals parent that it's done */

        printf("[%s %ld] Child about to signal parent\n",
                currTime("%T"), (long) getpid());
        if (kill(getppid(), SYNC_SIG) == -1)
            errExit("kill");
        
        printf("[%s %ld] Child about to wait for signal\n",
                currTime("%T"), (long) getpid());
        sigemptyset(&emptyMask);
        if (sigsuspend(&emptyMask) == -1 && errno != EINTR)
            errExit("sigsuspend");
        printf("[%s %ld] Child got signal\n", currTime("%T"), (long) getpid());


        /* Now child can do other things... */

        _exit(EXIT_SUCCESS);

    default: /* Parent */

        /* Parent may do some work here, and then waits for child to
           complete the required action */

        printf("[%s %ld] Parent about to wait for signal\n",
                currTime("%T"), (long) getpid());
        sigemptyset(&emptyMask);
        if (sigsuspend(&emptyMask) == -1 && errno != EINTR)
            errExit("sigsuspend");
        printf("[%s %ld] Parent got signal\n", currTime("%T"), (long) getpid());

        /* If required, return signal mask to its original state */

        if (sigprocmask(SIG_SETMASK, &origMask, NULL) == -1)
            errExit("sigprocmask");

        /* Parent carries on to do other things... */
        printf("[%s %ld] Parent about to signal child\n",
                currTime("%T"), (long) getpid());
        if (kill(childPid, SIGUSR2) == -1)
            errExit("kill");


        exit(EXIT_SUCCESS);
    }
}

