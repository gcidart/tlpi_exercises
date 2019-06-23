/*
 *Rewrite the programs in Listing 48-2 (svshm_xfr_writer.c) and Listing 48-3 (svshm_xfr_reader.c) to use POSIX shared memory objects instead of System V shared memory.
 * 
 * 
 * 
 * $ wc -c /etc/services 
 * 19183 /etc/services
 * $ ./pshm/svshm_xfr_writer < /etc/services &
 * [1] 6143
 * $ ./pshm/svshm_xfr_reader > test/outpsh.txt
 * Sent 19183 bytes (19 xfrs)
 * Received 19183 bytes (19 xfrs)
 * [1]+  Done                    ./pshm/svshm_xfr_writer < /etc/services
 * $ diff /etc/services test/outpsh.txt 
 * $
 */




#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "tlpi_hdr.h"


#if ! defined(__FreeBSD__) && ! defined(__OpenBSD__) && \
                ! defined(__sgi) && ! defined(__APPLE__)
                /* Some implementations already declare this union */

union semun {                   /* Used in calls to semctl() */
    int                 val;
    struct semid_ds *   buf;
    unsigned short *    array;
#if defined(__linux__)
    struct seminfo *    __buf;
#endif
};

#endif

#define SEM_KEY 0x1234          /* Key for shared memory segment */

#define OBJ_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
                                /* Permissions for our IPC objects */

#define WRITE_SEM 0             /* Writer has access to shared memory */
#define READ_SEM 1              /* Reader has access to shared memory */

#ifndef BUF_SIZE                /* Allow "cc -D" to override definition */
#define BUF_SIZE 1024           /* Size of transfer buffer */
#endif

struct shmseg {                 /* Defines structure of shared memory segment */
    int cnt;                    /* Number of bytes used in 'buf' */
    char buf[BUF_SIZE];         /* Data being transferred */
};

Boolean bsUseSemUndo = FALSE;
Boolean bsRetryOnEintr = TRUE;

/* Reserve semaphore (blocking), return 0 on success, or -1 with 'errno'
   set to EINTR if operation was interrupted by a signal handler */

static int                     /* Reserve semaphore - decrement it by 1 */
reserveSem(int semId, int semNum)
{
    struct sembuf sops;

    sops.sem_num = semNum;
    sops.sem_op = -1;
    sops.sem_flg = bsUseSemUndo ? SEM_UNDO : 0;

    while (semop(semId, &sops, 1) == -1)
        if (errno != EINTR || !bsRetryOnEintr)
            return -1;

    return 0;
}

static int                     /* Release semaphore - increment it by 1 */
releaseSem(int semId, int semNum)
{
    struct sembuf sops;

    sops.sem_num = semNum;
    sops.sem_op = 1;
    sops.sem_flg = bsUseSemUndo ? SEM_UNDO : 0;

    return semop(semId, &sops, 1);
}


int
main(int argc, char *argv[])
{
    int semid, xfrs, bytes, fd;
    struct shmseg *shmp;
    fd = shm_open("testsh", O_RDONLY,0);
    if (fd == -1)
        errExit("shm_open");



    /* Get IDs for semaphore set and shared memory created by writer */

    semid = semget(SEM_KEY, 0, 0);
    if (semid == -1)
        errExit("semget");


    /* Attach shared memory read-only, as we will only read */

    shmp = mmap(NULL, sizeof(int) + BUF_SIZE, PROT_READ, MAP_SHARED, fd, 0);


    if (shmp == (void *) -1)
        errExit("mmap");

    /* Transfer blocks of data from shared memory to stdout */

    for (xfrs = 0, bytes = 0; ; xfrs++) {
        if (reserveSem(semid, READ_SEM) == -1)          /* Wait for our turn */
            errExit("reserveSem");

        if (shmp->cnt == 0)                     /* Writer encountered EOF */
            break;
        bytes += shmp->cnt;

        if (write(STDOUT_FILENO, shmp->buf, shmp->cnt) != shmp->cnt)
            fatal("partial/failed write");

        if (releaseSem(semid, WRITE_SEM) == -1)         /* Give writer a turn */
            errExit("releaseSem");
    }

    if (munmap(shmp, sizeof(int) + BUF_SIZE ) == -1)
        errExit("munmap");

    /* Give writer one more turn, so it can clean up */

    if (releaseSem(semid, WRITE_SEM) == -1)
        errExit("releaseSem");

    fprintf(stderr, "Received %d bytes (%d xfrs)\n", bytes, xfrs);
    exit(EXIT_SUCCESS);
}


