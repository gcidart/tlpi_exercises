/*
 *
 * Write programs that verify the effect of the SA_RESETHAND and SA_NODEFER flags when establishing a signal handler with sigaction().
 *
 *
 * $ ./sig_receiver_nodefer_resethand &
 * [1] 12081
 * $ ./sig_receiver_nodefer_resethand: PID is 12081
 * 
 * $ ./sig_sender 12081 100000 10 2
 * ./sig_sender: sending signal 10 to process 12081 100000 times
 * In handler: Signal 10 caught 1 times
 * ERROR [ESRCH No such process] kill                             // Because of SA_RESETHAND handler() was called only once
 * [1]+  User defined signal 1   ./sig_receiver_nodefer_resethand // Program was terminated on the second time SIGUSR1 was caught
 * $ ./sig_receiver_nodefer_resethand &
 * [1] 12083
 * $ ./sig_receiver_nodefer_resethand: PID is 12083
 * 
 * $ ./sig_sender 12083 100000 12 2
 * ./sig_sender: sending signal 12 to process 12083 100000 times
 * In handler: Signal 12 caught 1 times
 * In handler: Signal 12 caught 2 times
 * In handler: Signal 12 caught 3 times
 * In handler: Signal 12 caught 4 times
 * In handler: Signal 12 caught 5 times
 * In handler: Signal 12 caught 6 times
 * In handler: Signal 12 caught 7 times
 * In handler: Signal 12 caught 8 times
 * In handler: Signal 12 caught 9 times
 * ./sig_sender: exiting
 * $ In handler: Signal 12 caught 10 times
 * ./sig_receiver_nodefer_resethand: signal 12 caught 10 times
 * 
 * [1]+  Done                    ./sig_receiver_nodefer_resethand
 * $ ./sig_receiver_nodefer_resethand &
 * [1] 12085
 * $ ./sig_receiver_nodefer_resethand: PID is 12085
 * 
 * $ ./sig_sender 12085 100000 26 2
 * ./sig_sender: sending signal 26 to process 12085 100000 times
 * ERROR [ESRCH No such process] kill
 * [1]+  Virtual timer expired   ./sig_receiver_nodefer_resethand  // Program was terminated on the first time SIGVTALARM was caught
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
    {
        sigCnt[sig]++;
	printf("In handler: Signal %d caught %d times\n", sig, sigCnt[sig]);
    }
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
    sa1.sa_flags = SA_NODEFER;
    sa2.sa_handler = &handler;
    sa2.sa_mask = saMask2;
    sa2.sa_flags = SA_NODEFER|SA_RESETHAND;
    sigaction(SIGINT, &sa1, NULL); /* Setup handler() as signal handler for SIGINT */
    sigaction(SIGUSR1, &sa2, NULL);/* Setup handler() as signal handler with SA_RESETHAND flag for SIGUSR1*/
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
