/*
 *
 *Listing 26-4 (make_zombie.c) uses a call to sleep() to allow the child process a chance to execute and terminate before the parent executes system(). 
 *This approach produces a theoretical race condition. Modify the program to eliminate the race condition by using signals to synchronize the parent and child.
 * $ ./procexec/make_zombie 
 * Parent PID=3884
 * Child (PID=3885) exiting
 *  3884 pts/0    00:00:00 make_zombie
 *  3885 pts/0    00:00:00 make_zombie <defunct>
 * After sending SIGKILL to zombie (PID=3885):
 *  3884 pts/0    00:00:00 make_zombie
 *  3885 pts/0    00:00:00 make_zombie <defunct>
 * 
 */

#include <signal.h>
#include <time.h>
#include <libgen.h>             /* For basename() declaration */

#include "tlpi_hdr.h"

#define BUF_SIZE 1000
#define CMD_SIZE 200


static void             /* Signal handler - does nothing but return */
handler(int sig)
{
}



int
main(int argc, char *argv[])
{
    char cmd[CMD_SIZE];
    pid_t childPid;
    sigset_t blockMask, origMask, emptyMask;
    struct sigaction sa;

    setbuf(stdout, NULL);       /* Disable buffering of stdout */

    printf("Parent PID=%ld\n", (long) getpid());
    sigemptyset(&blockMask);
    sigaddset(&blockMask, SIGCHLD);    /* Block signal */
    if (sigprocmask(SIG_BLOCK, &blockMask, &origMask) == -1)
        errExit("sigprocmask");

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        errExit("sigaction");


    switch (childPid = fork()) {
    case -1:
        errExit("fork");

    case 0:     /* Child: immediately exits to become zombie */
        printf("Child (PID=%ld) exiting\n", (long) getpid());

        _exit(EXIT_SUCCESS);

    default:    /* Parent */
        sigemptyset(&emptyMask);
        if (sigsuspend(&emptyMask) == -1 && errno != EINTR)
            errExit("sigsuspend");

        if (sigprocmask(SIG_SETMASK, &origMask, NULL) == -1)
            errExit("sigprocmask");

        snprintf(cmd, CMD_SIZE, "ps | grep %s", basename(argv[0]));
        system(cmd);            /* View zombie child */

        /* Now send the "sure kill" signal to the zombie */

        if (kill(childPid, SIGKILL) == -1)
            errMsg("kill");
        sleep(3);               /* Give child a chance to react to signal */
        printf("After sending SIGKILL to zombie (PID=%ld):\n", (long) childPid);
        system(cmd);            /* View zombie child again */

        exit(EXIT_SUCCESS);
    }
}

