/*
 *
 * Write a program that shows that when the disposition of a pending signal is changed to be SIG_IGN, the program never sees (catches) the signal.
 *
 *
 * $ ./sig_receiver_ign &
 * [1] 11840
 * $ ./sig_receiver_ign: PID is 11840
 * 
 * $ ./sig_sender 11840 1000000 12 2
 * ./sig_sender: sending signal 12 to process 11840 1000000 times
 * ./sig_sender: exiting
 * $ ./sig_receiver_ign: signal 12 caught 82 times
 * 
 * [1]+  Done                    ./sig_receiver_ign
 * $ ./sig_receiver_ign &
 * [1] 11842
 * $ ./sig_receiver_ign: PID is 11842
 * 
 * $ ./sig_sender 11842 1000000 10 2
 * ./sig_sender: sending signal 10 to process 11842 1000000 times
 * ./sig_sender: exiting
 * $ 
 * [1]+  Done                    ./sig_receiver_ign
 *
 *
 *
 * Note: http://man7.org/tlpi/code/online/dist/signals/sig_sender.c.html was used above for demonstration
 */

#include <signal.h>
#include "tlpi_hdr.h"

static int sigCnt[NSIG];                /* Counts deliveries of each signal */
static volatile sig_atomic_t gotSigint = 0;
                                        /* Set nonzero if SIGINT is delivered */

static void                    /* Print list of signals within a signal set */
printSigset(FILE *of, const char *prefix, const sigset_t *sigset)
{
    int sig, cnt;

    cnt = 0;
    for (sig = 1; sig < NSIG; sig++) {
        if (sigismember(sigset, sig)) {
            cnt++;
            fprintf(of, "%s%d (%s)\n", prefix, sig, strsignal(sig));
        }
    }

    if (cnt == 0)
        fprintf(of, "%s<empty signal set>\n", prefix);
}

static void
handler(int sig)
{
    if (sig == SIGINT)
        gotSigint = 1;
    else
        sigCnt[sig]++;
}

int
main(int argc, char *argv[])
{
    int n, numSecs;
    sigset_t pendingMask, blockingMask, emptyMask, saMask1, saMask2;
    struct sigaction sa1, sa2;

    printf("%s: PID is %ld\n", argv[0], (long) getpid());
    sigemptyset(&saMask1); 
    sigemptyset(&saMask2); 
    sa1.sa_handler = &handler;
    sa1.sa_mask = saMask1;
    sa1.sa_flags = 0;
    sa2.sa_handler = SIG_IGN;
    sa2.sa_mask = saMask2;
    sa2.sa_flags = 0;
    sigaction(SIGINT, &sa1, NULL); /* Setup handler() as signal handler for SIGINT */
    sigaction(SIGUSR1, &sa2, NULL);/*SIG_IGN for SIGUSR1*/
    sigaction(SIGUSR2, &sa1, NULL);/* Setup handler() as signal handler for SIGUSR2 */


    /* If a sleep time was specified, temporarily block all signals,
       sleep (while another process sends us signals), and then
       display the mask of pending signals and unblock all signals */

    if (argc > 1) {
        numSecs = getInt(argv[1], GN_GT_0, NULL);

        sigfillset(&blockingMask);
        if (sigprocmask(SIG_SETMASK, &blockingMask, NULL) == -1)
            errExit("sigprocmask");

        printf("%s: sleeping for %d seconds\n", argv[0], numSecs);
        sleep(numSecs);

        if (sigpending(&pendingMask) == -1)
            errExit("sigpending");

        printf("%s: pending signals are: \n", argv[0]);
        printSigset(stdout, "\t\t", &pendingMask);

        sigemptyset(&emptyMask);        /* Unblock all signals */
        if (sigprocmask(SIG_SETMASK, &emptyMask, NULL) == -1)
            errExit("sigprocmask");
    }

    while (!gotSigint)                  /* Loop until SIGINT caught */
        continue;

    for (n = 1; n < NSIG; n++)          /* Display number of signals received */
        if (sigCnt[n] != 0)
            printf("%s: signal %d caught %d time%s\n", argv[0], n,
                    sigCnt[n], (sigCnt[n] == 1) ? "" : "s");

    exit(EXIT_SUCCESS);
}
