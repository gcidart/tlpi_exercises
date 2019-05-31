/*
 *
 * Implement the siginterrupt() function described in Section 21.5 using sigaction(). 
 *
 *
 * $ ./t_siginterrupt 
 * ssss                    -----STDIN
 * ssss                    -----STDOUT
 * ^\Signal 3:Quit seen
 * ERROR [EINTR Interrupted system call] read
 * After enabling SA_RESTART flag
 * sss                    -----STDIN
 * sss                    -----STDOUT
 * ^\Signal 3:Quit seen
 * sssss                    -----STDIN
 * sssss                    -----STDOUT
 * aasda                    -----STDIN
 * aasda                    -----STDOUT
 * ^\Signal 3:Quit seen
 * ^CSignal 2:Interrupt seen
 * ERROR [EINTR Interrupted system call] read
 * ./t_siginterrupt: signal 3 caught 3 times
 *
 *
 */

#include <signal.h>
#include <fcntl.h>
#include "tlpi_hdr.h"



static int sigCnt[NSIG];                /* Counts deliveries of each signal */
static volatile sig_atomic_t gotSigint = 0;
                                        /* Set nonzero if SIGINT is delivered */


static void
handler(int sig)
{
    printf("Signal %d:%s seen\n", sig, strsignal(sig));
    if (sig == SIGINT)
        gotSigint = 1;
    else
	sigCnt[sig]++;
}

int 
t_siginterrupt(int sig, int flag);

int
main(int argc, char *argv[])
{
    int n, numRead;
    sigset_t  saMask;
    struct sigaction sa;
    char c;
    

    sigemptyset(&saMask); 
    sa.sa_handler = &handler;
    sa.sa_mask = saMask;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);

    while((numRead=read(STDIN_FILENO, &c, 1))!=-1)
    {
	if(write(STDOUT_FILENO, &c, numRead) != numRead)
	    fatal("couldn't write whole buffer on STDOUT");
    }
    if(numRead ==-1)
	errMsg("read");
    errno = 0;

    printf("After enabling SA_RESTART flag\n");
    t_siginterrupt(SIGQUIT, 0);


    while(((numRead=read(STDIN_FILENO, &c, 1))!=-1) && (!gotSigint))
    {
	if(write(STDOUT_FILENO, &c, numRead) != numRead)
	    fatal("couldn't write whole buffer on STDOUT");
    }
    if(numRead ==-1)
	errMsg("read");
    errno = 0;

    for (n = 1; n < NSIG; n++)          /* Display number of signals received */
        if (sigCnt[n] != 0)
            printf("%s: signal %d caught %d time%s\n", argv[0], n,
                    sigCnt[n], (sigCnt[n] == 1) ? "" : "s");

    exit(EXIT_SUCCESS);
}


int t_siginterrupt(int sig, int flag)
{
    struct sigaction sa;
    if(sigaction(sig, NULL, &sa)==-1)
	return -1;
    if(flag==0)
    {
        sa.sa_flags|= SA_RESTART;
    }
    else
    {
	sa.sa_flags&=(~SA_RESTART);
    }
    return sigaction(sig, &sa, NULL);
}


