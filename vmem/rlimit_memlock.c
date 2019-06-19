/*
 *Verify the effect of the RLIMIT_MEMLOCK resource limit by writing a program that sets a value for this limit and then attempts to lock more memory than the limit.
 * 
 * $ vmem/rlimit_memlock 
 * Trying to lock 16773120 bytes at 0x7fa631669000
 * Locked 16773120 bytes at 0x7fa631669000
 * Trying to lock 16781312 bytes at 0x7fa631669000
 * ERROR [ENOMEM Cannot allocate memory] mlock
 * 
 * 
 * 
 * 
 */
#include <sys/mman.h>
#include <sys/resource.h>
#include "tlpi_hdr.h"


int
main(int argc, char *argv[])
{
    char *addr;
    struct rlimit rlim;


    if (getrlimit(RLIMIT_MEMLOCK, &rlim)== -1)
        errExit("getrlimit");


    addr = mmap(NULL, rlim.rlim_cur, PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");

    printf("Trying to lock %ld bytes at %p\n",rlim.rlim_cur - sysconf(_SC_PAGESIZE), addr); 
    if (mlock(addr, rlim.rlim_cur - sysconf(_SC_PAGESIZE)) == -1)
            errExit("mlock");
    printf("Locked %ld bytes at %p\n",rlim.rlim_cur - sysconf(_SC_PAGESIZE), addr);
 
    if (munlock(addr, rlim.rlim_cur - sysconf(_SC_PAGESIZE)) == -1)
            errExit("munlock");

    printf("Trying to lock %ld bytes at %p\n",rlim.rlim_cur + sysconf(_SC_PAGESIZE), addr); 
    if (mlock(addr, rlim.rlim_cur + sysconf(_SC_PAGESIZE)) == -1)
            errExit("mlock");
    printf("Locked %ld bytes at %p\n",rlim.rlim_cur + sysconf(_SC_PAGESIZE), addr); 


   exit(EXIT_SUCCESS);
}
