/*
 *Write a program to verify the operation of the madvise() MADV_DONTNEED operation for a writable MAP_PRIVATE mapping.
 * 
 * $ vmem/madv_dontneed 
 * Character at 0x7f7336d83000 before madvise A
 * Character at 0x7f7336d83000 after madvise 
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


    addr = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");

    *addr = 'A';

    printf("Character at %p before madvise %c\n", addr, *addr);

    if(madvise(addr, sysconf(_SC_PAGESIZE), MADV_DONTNEED)==-1)
        errExit("madvise");
            

    printf("Character at %p after madvise %c\n", addr, *addr);


   exit(EXIT_SUCCESS);
}
