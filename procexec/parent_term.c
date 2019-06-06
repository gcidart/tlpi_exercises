/*
 *
 * Write a program to verify that when a child’s parent terminates, a call to getppid() returns 1 (the process ID of init).  
 * 
 * 
 * 
 * $ ./procexec/parent_term 
 * [11:58:28 2877] Parent started 
 * [11:58:28 2878] Child started - Parent:2877
 * [11:58:30 2877] Parent about to exit
 * $ [11:58:38 2878] Child after sleeping - Parent:1368
 * 
 * $ ps -ax | grep 1368
 *  1368 ?        Ss     0:00 /lib/systemd/systemd --user
 *  2880 pts/0    S+     0:00 grep --color=auto 1368
 *  
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


int
main(int argc, char *argv[])
{
    pid_t childPid;

    setbuf(stdout, NULL);               /* Disable buffering of stdout */

    printf("[%s %ld] Parent started \n",
        currTime("%T"), (long) getpid());

    switch (childPid = fork()) {
    case -1:
        errExit("fork");

    case 0: /* Child */

        /* Child does some required action here... */

        printf("[%s %ld] Child started - Parent:%ld\n",
                currTime("%T"), (long) getpid(), (long) getppid());
        sleep(10);               /* Simulate time spent doing some work */

        printf("[%s %ld] Child after sleeping - Parent:%ld\n",
                currTime("%T"), (long) getpid(), (long) getppid());

        _exit(EXIT_SUCCESS);

    default: /* Parent */

        sleep(2);
        printf("[%s %ld] Parent about to exit\n",
                currTime("%T"), (long) getpid());


        exit(EXIT_SUCCESS);
    }
}

