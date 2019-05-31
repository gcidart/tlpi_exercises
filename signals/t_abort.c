/*
 *
 * Implement abort()
 *
 * 
 *
 * $ ./signals/t_abort 
 * Raising SIGABRT using raise()
 * Inside signal handler for SIGABRT
 * Calling SIGABRT using t_abort()
 * Raising SIGABRT using raise() inside t_abort()
 * Inside signal handler for SIGABRT
 * Changing handler for SIGABRT to SIG_DFL
 * Raising SIGABRT again using raise() inside t_abort()
 * Aborted (core dumped)
 *
 *
 * 
 *
 */

#include <signal.h>
#include "tlpi_hdr.h"


static void
handler(int sig)
{
    printf("Inside signal handler for SIGABRT\n");
    
}

static void
t_abort()
{
    struct sigaction sa;
    printf("Raising SIGABRT using raise() inside t_abort()\n");
    raise(SIGABRT);
    printf("Changing handler for SIGABRT to SIG_DFL\n");
    sigaction(SIGABRT, NULL, &sa);
    sa.sa_handler = SIG_DFL;
    sigaction(SIGABRT, &sa, NULL);
    printf("Raising SIGABRT again using raise() inside t_abort()\n");
    raise(SIGABRT);
}


int
main(int argc, char *argv[])
{
    struct sigaction sa;

    sigemptyset(&sa.sa_mask); 
    sa.sa_handler = &handler;
    sa.sa_flags = 0;
    sigaction(SIGABRT, &sa, NULL);
    printf("Raising SIGABRT using raise()\n");
    raise(SIGABRT);
    printf("Calling SIGABRT using t_abort()\n");
    t_abort();



    exit(EXIT_SUCCESS);
}
