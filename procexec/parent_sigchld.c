/*
 *
 *Suppose that a parent process has established a handler for SIGCHLD and also blocked this signal.
 *Subsequently, one of its children exits, and the parent then does a wait() to collect the child’s status.
 *What happens when the parent unblocks SIGCHLD? Write a program to verify your answer.
 *What is the relevance of the result for a program calling the system() function?
 * 
 * 
 * $ ./procexec/parent_sigchld 
 * Child exiting
 * After wait
 * Inside handler with sig 17
 * Parent about to exit
 * 
*/

#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tlpi_hdr.h"



static void             /* Signal handler  */
handler(int sig)
{
    printf("Inside handler with sig %d\n", sig);
}

int
main(int argc, char *argv[])
{
    pid_t childPid;
    sigset_t blockMask, origMask;
    struct sigaction sa;

    setbuf(stdout, NULL);               /* Disable buffering of stdout */

    sigemptyset(&blockMask);
    sigaddset(&blockMask, SIGCHLD);    /* Block signal */
    if (sigprocmask(SIG_BLOCK, &blockMask, &origMask) == -1)
        errExit("sigprocmask");

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        errExit("sigaction");

    switch (childPid = fork()) {
    case -1:
        errExit("fork");

    case 0: /* Child */


        printf("Child exiting\n");

        _exit(EXIT_SUCCESS);

    default: /* Parent */

        wait(NULL);
        printf("After wait\n"); 
        if (sigprocmask(SIG_SETMASK, &origMask, NULL) == -1)
            errExit("sigprocmask");

        /* Parent carries on to do other things... */
        printf("Parent about to exit\n");


        exit(EXIT_SUCCESS);
    }
}

