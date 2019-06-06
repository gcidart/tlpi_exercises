/*
 *
 * Replace the use of waitpid() with waitid() in Listing 26-3 (child_status.c). 
 * The call to printWaitStatus() will need to be replaced by code that prints relevant fields from the siginfo_t structure returned by waitid().   
 *
 *
 * $ ./procexec/child_status 23
 * Child started with PID = 3684
 * waitid() returned: PID=3684; status=0x0017 (0,23)
 * child exited, status=23
 * $ ./procexec/child_status &
 * [1] 3685
 * $ Child started with PID = 3686
 * 
 * $ kill -STOP 3686
 * $ waitid() returned: PID=3686; status=0x0013 (0,19)
 * child stopped by signal 19 (Stopped (signal))
 * 
 * $ kill -CONT 3686
 * $ waitid() returned: PID=3686; status=0x0012 (0,18)
 * child continued
 * 
 * $ kill -ABRT 3686
 * $ waitid() returned: PID=3686; status=0x0006 (0,6)
 * child aborted by signal 6 (Aborted)
 * 
 * [1]+  Done                    ./procexec/child_status
 * 
 */
#include <string.h>
#include <sys/wait.h>
#include "tlpi_hdr.h"




int
main(int argc, char *argv[])
{
    pid_t childPid;
    siginfo_t si;

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [exit-status]\n", argv[0]);

    switch (childPid = fork()) {
    case -1: errExit("fork");

    case 0:             /* Child: either exits immediately with given
                           status or loops waiting for signals */
        printf("Child started with PID = %ld\n", (long) getpid());
        if (argc > 1)                   /* Status supplied on command line? */
        {
            exit(atoi(argv[1]));
        }
        else                            /* Otherwise, wait for signals */
            for (;;)
                pause();
        exit(EXIT_FAILURE);             /* Not reached, but good practice */

    default:            /* Parent: repeatedly wait on child until it
                           either exits or is terminated by a signal */
        for (;;) {
            if( waitid(P_PID, childPid, &si, WEXITED | WSTOPPED | WCONTINUED ) ==-1)
                errExit("waitid");

            /* Print status in hex, and as separate decimal bytes */

            printf("waitid() returned: PID=%ld; status=0x%04x (%d,%d)\n",
                    (long) childPid,
                    (unsigned int) si.si_status, si.si_status >> 8, si.si_status & 0xff);
            if (si.si_code==CLD_EXITED) {
                printf("child exited, status=%d\n", si.si_status & 0xff);

            } else if (si.si_code==CLD_KILLED) {
                printf("child killed by signal %d (%s)\n",
                    si.si_status & 0xff, strsignal(si.si_status & 0xff));
            }
            else if (si.si_code==CLD_DUMPED){
                printf("child aborted by signal %d (%s)\n",
                    si.si_status & 0xff, strsignal(si.si_status & 0xff));
            } else if (si.si_code==CLD_STOPPED) {
                printf("child stopped by signal %d (%s)\n",
                    si.si_status & 0xff, strsignal(si.si_status));

            } else if (si.si_code==CLD_CONTINUED) {
                printf("child continued\n");

            } else {            /* Should never happen */
                printf("what happened to this child? (status=%x)\n",
                    (unsigned int) si.si_status);
            }


            if (si.si_code==CLD_EXITED || si.si_code==CLD_KILLED || si.si_code == CLD_DUMPED)
                exit(EXIT_SUCCESS);
        }
    }
}
