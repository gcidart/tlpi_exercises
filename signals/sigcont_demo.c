/*
 *
 * Section 22.2 noted that if a stopped process that has established a handler for and blocked SIGCONT is later resumed as a consequence of receiving a SIGCONT, 
 * then the handler is invoked only when SIGCONT is unblocked. Write a program to verify this. 
 * Recall that a process can be stopped by typing the terminal suspend character (usually Control-Z) and 
 * can be sent a SIGCONT signal using the command kill –CONT (or implicitly, using the shell fg command). *
 * 
 *
 * $ signals/sigcont_demo &
 * [1] 3303
 * $ signals/sigcont_demo: PID is 3303
 * 
 * $ kill -CONT 3303
 * $ kill -CONT 3303
 * $ kill -USR1 3303
 * $ SIGCONT unblocking
 * SIGCONT received
 * 
 * [1]+  Done                    signals/sigcont_demo
  *
 * 
 *
 */

#include <signal.h>
#include "tlpi_hdr.h"

static volatile sig_atomic_t gotSigint = 0;

static void
handler(int sig)
{
    sigset_t mask;
    if(sig==SIGUSR1)
    {
	printf("SIGCONT unblocking\n");
	sigemptyset(&mask);
	sigaddset(&mask, SIGCONT);
        sigprocmask(SIG_UNBLOCK,&mask, NULL);
    }
    else if(sig==SIGCONT)
    {
	printf("SIGCONT received\n");
	gotSigint = 1;

    }
	
    
}




int
main(int argc, char *argv[])
{
    sigset_t mask;
    struct sigaction sa;

    printf("%s: PID is %ld\n", argv[0], (long) getpid());
    sigemptyset(&sa.sa_mask); 
    sa.sa_handler = &handler;
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGCONT, &sa, NULL);
    sigemptyset(&mask);
    sigaddset(&mask, SIGCONT);
    sigprocmask(SIG_BLOCK,&mask, NULL);

    while (!gotSigint)                  /* Loop until SIGCONT caught */
        continue;




    exit(EXIT_SUCCESS);
}
