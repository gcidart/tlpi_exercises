/*
 *Rewrite the programs in Listing 48-2 (svshm_xfr_writer.c, page 1003) and Listing 48-3 (svshm_xfr_reader.c, page 1005) to use a shared memory mapping instead of System V shared memory.
 * 
 * 
 * 
 * $ dd if=/dev/zero of=test/shm.txt bs=1 count=4096
 * 4096+0 records in
 * 4096+0 records out
 * 4096 bytes (4.1 kB, 4.0 KiB) copied, 0.0129464 s, 316 kB/s
 * $ wc -c /etc/services
 * 19183 /etc/services
 * $ ./mmap/svshm_xfr_writer < /etc/services &
 * [1] 5019
 * $ ./mmap/svshm_xfr_reader > test/out.txt
 * Sent 19183 bytes (19 xfrs)
 * Received 19183 bytes (19 xfrs)
 * [1]+  Done                    ./mmap/svshm_xfr_writer < /etc/services
 * $ diff /etc/services test/out.txt 
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
    fd = open("test/shm.txt", O_RDONLY);
    if (fd == -1)
        errExit("open");



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


