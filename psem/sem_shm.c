/*
 *Rewrite the programs in Listing 48-2 and Listing 48-3 (Section 48.4) as a threaded application,
with the two threads passing data to each other via a global buffer, and using POSIX semaphores for synchronization. 
 * 
 * $ wc -c /etc/services
 * 19183 /etc/services
 * $ psem/sem_shm < /etc/services > test/outs.txt
 * Sent 19183 bytes (19 xfrs)
 * Received 19183 bytes (19 xfrs)
 * $ diff /etc/services test/outs.txt 
 * $  
*/


#include <semaphore.h>
#include <pthread.h>
#include <sys/mman.h>
#include "tlpi_hdr.h"



#ifndef BUF_SIZE                
#define BUF_SIZE 1024           /* Size of transfer buffer */
#endif

struct shmseg {                 /* Defines structure of shared memory segment */
    int cnt;                    /* Number of bytes used in 'buf' */
    char buf[BUF_SIZE];         /* Data being transferred */
};

static sem_t wsem, rsem;
struct shmseg *shmp;


static void *                  
readFunc(void *arg)
{
    int xfrs, bytes;
    /* Transfer blocks of data from shared memory to stdout */

    for (xfrs = 0, bytes = 0; ; xfrs++) {
        if (sem_wait(&rsem) == -1)          /* Wait for our turn */
            errExit("sem_wait");

        if (shmp->cnt == 0)                     /* Writer encountered EOF */
            break;
        bytes += shmp->cnt;

        if (write(STDOUT_FILENO, shmp->buf, shmp->cnt) != shmp->cnt)
            fatal("partial/failed write");

        if (sem_post(&wsem) == -1)         /* Give writer a turn */
            errExit("sem_post");
    }
    if (sem_post(&wsem) == -1)
        errExit("sem_post");

    fprintf(stderr, "Received %d bytes (%d xfrs)\n", bytes, xfrs);

    return NULL;
}



static void *
writeFunc(void *arg)
{
    int xfrs, bytes;
    /* Transfer blocks of data from stdin to shared memory */

    for (xfrs = 0, bytes = 0; ; xfrs++, bytes += shmp->cnt) {
        if (sem_wait(&wsem) == -1)          /* Wait for our turn */
            errExit("sem_wait");

        shmp->cnt = read(STDIN_FILENO, shmp->buf, BUF_SIZE);
        if (shmp->cnt == -1)
            errExit("read");

        if (sem_post(&rsem) == -1)         /* Give reader a turn */
            errExit("sem_post");

        /* Have we reached EOF? We test this after giving the reader
           a turn so that it can see the 0 value in shmp->cnt. */

        if (shmp->cnt == 0)
            break;
    }

    /* Wait until reader has let us have one more turn. We then know
       reader has finished */

    if (sem_wait(&wsem) == -1)      
            errExit("sem_post");

    fprintf(stderr, "Sent %d bytes (%d xfrs)\n", bytes, xfrs);
    return NULL;

}

int
main(int argc, char *argv[])
{
    pthread_t t1, t2;
    int s;
        
    if (sem_init(&wsem, 0, 1) == -1)
        errExit("sem_init");

    if (sem_init(&rsem, 0, 0) == -1)
        errExit("sem_init");
    

    /* Attach shared memory read-only, as we will only read */

    shmp = mmap(NULL, sizeof(int) + BUF_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);


    if (shmp == (void *) -1)
        errExit("mmap");

    /* Create two threads that increment 'glob' */

    s = pthread_create(&t1, NULL, readFunc, NULL);
    if (s != 0)
        errExitEN(s, "pthread_create");
    s = pthread_create(&t2, NULL, writeFunc, NULL);
    if (s != 0)
        errExitEN(s, "pthread_create");

    /* Wait for threads to terminate */

    s = pthread_join(t1, NULL);
    if (s != 0)
        errExitEN(s, "pthread_join");
    s = pthread_join(t2, NULL);
    if (s != 0)
        errExitEN(s, "pthread_join");

    if (munmap(shmp, sizeof(int) + BUF_SIZE ) == -1)
        errExit("munmap");

    exit(EXIT_SUCCESS);
}


