/*
 *Modify the program in Listing 53-3 (psem_wait.c) to use sem_timedwait() instead of sem_wait().
 *The program should take an additional command-line argument that specifies a (relative) number of seconds to be used as the timeout for the sem_timedwait() call.
 * 
 * $ psem/psem_create -c /demo 600 0
 * $ time psem/psem_timedwait /demo 10
 * ERROR [ETIMEDOUT Connection timed out] sem_wait
 * 
 * real	0m10.004s
 * user	0m0.001s
 * sys	0m0.000s
 * 
 *http://man7.org/tlpi/code/online/dist/psem/psem_create.c.html
 */
#include <semaphore.h>
#include <time.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    sem_t *sem;
    struct timespec ct;

    

    if (argc < 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s sem-name time\n", argv[0]);

    if(clock_gettime(CLOCK_REALTIME, &ct)==-1)
        errExit("clk_gettime");

    ct.tv_sec+= atol(argv[2]);

    sem = sem_open(argv[1], 0);
    if (sem == SEM_FAILED)
        errExit("sem_open");

    if (sem_timedwait(sem, &ct) == -1)
        errExit("sem_wait");

    printf("%ld sem_wait() succeeded\n", (long) getpid());
    exit(EXIT_SUCCESS);
}

