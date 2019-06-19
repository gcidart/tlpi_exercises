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

static int                     /* Initialize semaphore to 1 (i.e., "available") */
initSemAvailable(int semId, int semNum)
{
    union semun arg;

    arg.val = 1;
    return semctl(semId, semNum, SETVAL, arg);
}

static int                     /* Initialize semaphore to 0 (i.e., "in use") */
initSemInUse(int semId, int semNum)
{
    union semun arg;

    arg.val = 0;
    return semctl(semId, semNum, SETVAL, arg);
}


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
    int semid,  bytes, xfrs, fd;
    struct shmseg *shmp;
    union semun dummy;

    fd = open("test/shm.txt", O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    if (fd == -1)
        errExit("open");


    /* Create set containing two semaphores; initialize so that
       writer has first access to shared memory. */

    semid = semget(SEM_KEY, 2, IPC_CREAT | OBJ_PERMS);
    if (semid == -1)
        errExit("semget");

    if (initSemAvailable(semid, WRITE_SEM) == -1)
        errExit("initSemAvailable");
    if (initSemInUse(semid, READ_SEM) == -1)
        errExit("initSemInUse");

    /* Create shared memory; attach at address chosen by system */

    shmp = mmap(NULL, sizeof(int) + BUF_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);


    if (shmp == (void *) -1)
        errExit("mmap");

    /* Transfer blocks of data from stdin to shared memory */

    for (xfrs = 0, bytes = 0; ; xfrs++, bytes += shmp->cnt) {
        if (reserveSem(semid, WRITE_SEM) == -1)         /* Wait for our turn */
            errExit("reserveSem");

        shmp->cnt = read(STDIN_FILENO, shmp->buf, BUF_SIZE);
        if (shmp->cnt == -1)
            errExit("read");

        if (releaseSem(semid, READ_SEM) == -1)          /* Give reader a turn */
            errExit("releaseSem");

        /* Have we reached EOF? We test this after giving the reader
           a turn so that it can see the 0 value in shmp->cnt. */

        if (shmp->cnt == 0)
            break;
    }

    /* Wait until reader has let us have one more turn. We then know
       reader has finished, and so we can delete the IPC objects. */

    if (reserveSem(semid, WRITE_SEM) == -1)
        errExit("reserveSem");

    if (semctl(semid, 0, IPC_RMID, dummy) == -1)
        errExit("semctl");
    if (munmap(shmp, sizeof(int) + BUF_SIZE ) == -1)
        errExit("munmap");

    fprintf(stderr, "Sent %d bytes (%d xfrs)\n", bytes, xfrs);
    exit(EXIT_SUCCESS);
}
