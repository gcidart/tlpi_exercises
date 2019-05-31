/*
 *
 * As noted in Section 20.3, sigaction() is more portable than signal() for establishing a signal handler.
 * Replace the use of signal() by sigaction() in the program in Listing 20-7 (sig_receiver.c).
 *
 *
 *
 * $ ./sig_receiver &
 * [1] 11392
 * $ ./sig_receiver: PID is 11392
 * 
 * $ ./sig_sender 11392 1000000 10 2
 * ./sig_sender: sending signal 10 to process 11392 1000000 times
 * ./sig_sender: exiting
 * $ ./sig_receiver: signal 10 caught 81 times
 *
 *
 *
 * $ ./sig_receiver 15 &
 * [1] 11398
 * $ ./sig_receiver: PID is 11398
 * ./sig_receiver: sleeping for 15 seconds
 * 
 * $ ./sig_sender 11398 1000000 10 2
 * ./sig_sender: sending signal 10 to process 11398 1000000 times
 * ./sig_sender: exiting
 * $ ./sig_receiver: pending signals are: 
 * 		2 (Interrupt)
 * 		10 (User defined signal 1)
 * ./sig_receiver: signal 10 caught 1 time
 * 
 * [1]+  Done                    ./sig_receiver 15
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
    sigset_t pendingMask, blockingMask, emptyMask, saMask;
    struct sigaction sa;

    printf("%s: PID is %ld\n", argv[0], (long) getpid());
    sigemptyset(&saMask); 
    sa.sa_handler = &handler;
    sa.sa_mask = saMask;
    sa.sa_flags = 0;
    for (n = 1; n < NSIG; n++)          /* Same handler for all signals */
	sigaction(n, &sa, NULL);

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
