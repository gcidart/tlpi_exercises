/*
 *Write programs to verify that the SIGBUS and SIGSEGV signals are delivered in the circumstances described in Section 49.4.3. 
 *
 *
 * $ ./mmap/mmap_boundary test/mb.txt 9000
 * File size 2200
 * Byte 0: 0 
 * Byte 9000: 0 
 * $ ./mmap/mmap_boundary test/mb.txt 4100
 * File size 2200
 * Byte 0: 0 
 * Signal received:Bus error
 * Terminated
 * $ ./mmap/mmap_boundary test/mb.txt 2000
 * File size 2200
 * Byte 0: 0 
 * Byte 2000: 0 
 * $ ./mmap/mmap_boundary test/mb.txt 3000
 * File size 2200
 * Byte 0: 0 
 * Byte 3000: 0 
 * 
 * 
*/
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "tlpi_hdr.h"

static void
handler(int sig)
{
    printf("Signal received:%s\n", strsignal(sig));	
    raise(SIGTERM);
}


int
main(int argc, char *argv[])
{
    char *addr;
    int fd;
    struct stat sb;
    struct sigaction sa;

    if (argc != 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file location\n", argv[0]);

    sigemptyset(&sa.sa_mask); 
    sa.sa_handler = &handler;
    sa.sa_flags = 0;
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);


    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        errExit("open");

    /* Obtain the size of the file and use it to specify the size of
       the mapping and the size of the buffer to be written */

    if (fstat(fd, &sb) == -1)
        errExit("fstat");

    printf("File size %ld\n", sb.st_size);

    
    addr = mmap(NULL, 8192, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");

    printf("Byte 0: %d \n", *addr);
    printf("Byte %d: %d \n",atoi(argv[2]), *(addr+atoi(argv[2])));

    


    exit(EXIT_SUCCESS);
}
