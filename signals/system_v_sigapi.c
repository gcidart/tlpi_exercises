/*
 * Implement the System V functions sigset(), sighold(), sigrelse(), sigignore(), and sigpause() using the POSIX signal API.
 *
 *
 *
 * $ ./system_v_sigapi &
 * [1] 4374
 * $ ./system_v_sigapi: PID is 4374
 * 
 * $ kill -WINCH 4374
 * $ kill -USR1 4374
 * $ Inside signal handler: Signal 10 received
 * 
 * $ kill -USR2 4374
 * $ Inside signal handler: Signal 12 received
 * Ignoring SIGUSR2
 * 
 * $ kill -USR2 4374
 * $ kill -INT 4374
 * $ Inside signal handler: Signal 2 received
 * Inside signal handler: Signal 28 received
 * 
 * $ kill -VTALRM 4374
 * $ Inside signal handler: Signal 26 received
 * ./system_v_sigapi: signal 2 caught 1 time
 * ./system_v_sigapi: signal 10 caught 1 time
 * ./system_v_sigapi: signal 12 caught 1 time
 * ./system_v_sigapi: signal 26 caught 1 time
 * ./system_v_sigapi: signal 28 caught 1 time
 * 
 * [1]+  Done                    ./system_v_sigapi
 *
 */




#include <signal.h>
#include "tlpi_hdr.h"

typedef void (*sighandler_t)(int);

static sighandler_t
t_sigset(int sig, sighandler_t handler)
{
    struct sigaction newDisp, prevDisp;

    newDisp.sa_handler = handler;
    sigemptyset(&newDisp.sa_mask);

    if (sigaction(sig, &newDisp, &prevDisp) == -1)
        return SIG_ERR;
    else
        return prevDisp.sa_handler;
}

static int
t_sighold(int sig)
{
    sigset_t mask;
    if(sigprocmask(SIG_BLOCK, NULL, &mask)==-1)
        return -1;
    if(sigaddset(&mask, sig)==-1)
	return -1;
    return sigprocmask(SIG_BLOCK, &mask, NULL);
}

static int
t_sigrelse(int sig)
{
    sigset_t mask;
    if(sigprocmask(SIG_UNBLOCK, NULL, &mask)==-1)
        return -1;
    sigaddset(&mask, sig);
    return sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

static int
t_sigignore(int sig)
{
    struct sigaction sa;
    if(sigaction(sig, NULL, &sa)==-1)
        return -1;
    sa.sa_handler = SIG_IGN;
    return sigaction(sig, &sa, NULL);
}

static int
t_sigpause(int sig)
{
    sigset_t blockedMask;
    sigfillset(&blockedMask);
    sigdelset(&blockedMask, sig);
    return sigsuspend(&blockedMask);
}

static int sigCnt[NSIG];                /* Counts deliveries of each signal */
static volatile sig_atomic_t gotSigint = 0;

static void
handler(int sig)
{
    printf("Inside signal handler: Signal %d received\n", sig);
    if (sig == SIGINT)
        gotSigint = 1;
    else if(sig== SIGUSR2)
    {
	t_sigignore(SIGUSR2);
	printf("Ignoring SIGUSR2\n");
    }
    sigCnt[sig]++;

    
}


int
main(int argc, char *argv[])
{
    int n;

    t_sigset(SIGUSR1, handler);
    t_sigset(SIGUSR2, handler);
    t_sigset(SIGINT, handler);
    t_sigset(SIGWINCH, handler);
    t_sigset(SIGVTALRM, handler);
    
    t_sighold(SIGWINCH); 

    printf("%s: PID is %ld\n", argv[0], (long) getpid());

    while (!gotSigint)                  /* Loop until SIGINT caught */
        continue;
    t_sigrelse(SIGWINCH);
    gotSigint = 0;
    t_sigpause(SIGVTALRM);
    for (n = 1; n < NSIG; n++)          /* Display number of signals received */
        if (sigCnt[n] != 0)
            printf("%s: signal %d caught %d time%s\n", argv[0], n,
                    sigCnt[n], (sigCnt[n] == 1) ? "" : "s");





    exit(EXIT_SUCCESS);
}
