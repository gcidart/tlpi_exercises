/*
 *Write a program, analogous to cp(1), that uses mmap() and memcpy() calls (instead of read() or write()) to copy a source file to a destination file.
 *(Use fstat() to obtain the size of the input file, which can then be used to size the required memory mappings, and use ftruncate() to set the size of the output file.)
 * 
 * $ ./mmap/t_cp test/test.txt test/test_cp.txt
 * $ diff test/test.txt test/test_cp.txt 
 * 
 * 
*/
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    char *saddr, *daddr;
    int sfd, dfd;
    struct stat sb;

    if (argc != 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s src dst\n", argv[0]);

    sfd = open(argv[1], O_RDONLY);
    if (sfd == -1)
        errExit("open");

    /* Obtain the size of the file and use it to specify the size of
       the mapping and the size of the buffer to be written */

    if (fstat(sfd, &sb) == -1)
        errExit("fstat");

    /* Handle zero-length file specially, since specifying a size of
       zero to mmap() will fail with the error EINVAL */

    if (sb.st_size == 0)
        exit(EXIT_SUCCESS);

    saddr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, sfd, 0);
    if (saddr == MAP_FAILED)
        errExit("mmap");

    dfd = open(argv[2], O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
    if (dfd == -1)
        errExit("open");

    if (ftruncate(dfd, sb.st_size) == -1)
        errExit("ftruncate");


    daddr = mmap(NULL, sb.st_size, PROT_WRITE, MAP_SHARED, dfd, 0);
    if (daddr == MAP_FAILED)
        errExit("mmap");

    memcpy(daddr,saddr, sb.st_size);

    if (msync(daddr, sb.st_size, MS_SYNC) == -1)
        errExit("msync");



    exit(EXIT_SUCCESS);
}
