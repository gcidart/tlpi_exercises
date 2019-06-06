/*
 *
 * Suppose that we have three processes related as grandparent, parent, and child, and 
 * that the grandparent doesn’t immediately perform a wait() after the parent exits, so that the parent becomes a zombie. 
 * When do you expect the grandchild to be adopted by init (so that getppid() in the grandchild returns 1): 
 * after the parent terminates or after the grandparent does a wait()? Write a program to verify your answer.  
 * 
 * 
 * 
 * $ ./procexec/grandchild 
 * [12:37:10 3212] Grandparent started with ppid:2753
 * [12:37:10 3213] Parent/child started with ppid:3212
 * [12:37:12 3214] Grandchild started with ppid:3213
 * [12:37:14 3213] Parent about to exit with ppid:3212
 * [12:37:22 3214] Grandchild after sleeping with ppid:1368
 * [12:37:25 3212] Grandparent about to exit with ppid:2753
 * $ ps -ax | grep 1368
 *  1368 ?        Ss     0:00 /lib/systemd/systemd --user *  
 */

#include <signal.h>
#include <sys/wait.h>
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


int
main(int argc, char *argv[])
{
    pid_t childPid, gcPid;

    setbuf(stdout, NULL);               /* Disable buffering of stdout */

    printf("[%s %ld] Grandparent started with ppid:%ld\n",
        currTime("%T"), (long) getpid(), (long) getppid());

    switch (childPid = fork()) {
    case -1:
        errExit("fork");

    case 0: /* Child */


        printf("[%s %ld] Parent/child started with ppid:%ld\n",
                currTime("%T"), (long) getpid(), (long) getppid());
        sleep(2);               /* Simulate time spent doing some work */
        switch (gcPid = fork()) {
        case -1:
            errExit("fork");
        case 0: /* Grandchild */
            printf("[%s %ld] Grandchild started with ppid:%ld\n",
                    currTime("%T"), (long) getpid(), (long) getppid());
            sleep(10);               /* Simulate time spent doing some work */

            printf("[%s %ld] Grandchild after sleeping with ppid:%ld\n",
                currTime("%T"), (long) getpid(), (long) getppid());
            _exit(EXIT_SUCCESS);

        default:
            sleep(2);
            printf("[%s %ld] Parent about to exit with ppid:%ld\n",
                    currTime("%T"), (long) getpid(), (long) getppid());
        

            _exit(EXIT_SUCCESS);
        }       

    default: /* Parent */

        sleep(15);
        printf("[%s %ld] Grandparent about to exit with ppid:%ld\n",
                currTime("%T"), (long) getpid(), (long) getppid());


        exit(EXIT_SUCCESS);
    }
}

