/*
 *
 * Section 22.10 stated that accepting signals using sigwaitinfo() is faster than the use of a signal handler plus sigsuspend(). 
 * The program signals/sig_speed_sigsuspend.c, supplied in the source code distribution for this book, 
 * uses sigsuspend() to alternately send signals back and forward between a parent and a child process.
 * Time the operation of this program to exchange one million signals between the two processes.
 * (The number of signals to exchange is provided as a command-line argument to the program.) 
 * Create a modified version of the program that instead uses sigwaitinfo(), and time that version. 
 * What is the speed difference between the two programs?
 *
 *
 *
 *
 * $ time ./sig_speed_sigwaitinfo 1000000
 * 
 * real	0m3.991s
 * user	0m0.294s
 * sys	0m1.680s
 * $ time ./sig_speed_sigsuspend 1000000
 * 
 * real	0m5.373s
 * user	0m0.519s
 * sys	0m2.142s
 */


#include <signal.h>
#include "tlpi_hdr.h"


#define TESTSIG SIGUSR1

int
main(int argc, char *argv[])
{
    int numSigs, scnt;
    pid_t childPid;
    sigset_t blockedMask ;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s num-sigs\n", argv[0]);

    numSigs = atoi(argv[1]);

    /* Block the signal before fork(), so that the child doesn't manage
       to send it to the parent before the parent is ready to catch it */

    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, TESTSIG);
    if (sigprocmask(SIG_SETMASK, &blockedMask, NULL) == -1)
        errExit("sigprocmask");


    switch (childPid = fork()) {
    case -1: errExit("fork");

    case 0:     /* child */
        for (scnt = 0; scnt < numSigs; scnt++) {
            if (kill(getppid(), TESTSIG) == -1)
                errExit("kill");
            if (sigwaitinfo(&blockedMask,NULL) == -1 && errno != EINTR)
                    errExit("sigwaitinfo");
        }
        exit(EXIT_SUCCESS);

    default: /* parent */
        for (scnt = 0; scnt < numSigs; scnt++) {
            if (sigwaitinfo(&blockedMask,NULL) == -1 && errno != EINTR)
                    errExit("sigwaitinfo");
            if (kill(childPid, TESTSIG) == -1)
                errExit("kill");
        }
        exit(EXIT_SUCCESS);
    }
}
