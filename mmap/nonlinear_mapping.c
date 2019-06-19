/*
 *Write a program that uses the MAP_FIXED technique described in Section 49.10 to create a nonlinear mapping similar to that shown in Figure 49-5 
 * 
 * 
 * $ ./mmap/nonlinear_mapping test/nlm
 * Byte 0 before non linear mapping: 0000 
 * Byte 4000 before non linear mapping: 1000 
 * Byte 8000 before non linear mapping: 2000 
 * Byte 12000 before non linear mapping: 3000 
 * Byte 0 after non linear mapping: 2048 
 * Byte 4000 after non linear mapping: 3048 
 * Byte 8000 after non linear mapping: 2000 
 * Byte 12000 after non linear mapping: 0952 
 * 
*/
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "tlpi_hdr.h"



int
main(int argc, char *argv[])
{
    char *addr, *tmp;
    char buf[4];
    int fd;
    FILE *file;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file \n", argv[0]);



    fd = open(argv[1], O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    if (fd == -1)
        errExit("open");
    file = fdopen(fd, "w");

    for(int i=0; i < 4096*3/4; i++)
        fprintf(file, "%04d", i);
    fsync(fd);
    

    
    addr = mmap(NULL, 4096*3, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");

    memcpy(&buf, addr, 4);
    printf("Byte 0 before non linear mapping: %s \n", buf);
    memcpy(&buf, (addr+4000), 4);
    printf("Byte 4000 before non linear mapping: %s \n", buf);
    memcpy(&buf, (addr+8000), 4);
    printf("Byte 8000 before non linear mapping: %s \n", buf);
    memcpy(&buf, (addr+12000), 4);
    printf("Byte 12000 before non linear mapping: %s \n", buf);
    
    tmp = mmap(addr, 4096, PROT_READ, MAP_SHARED|MAP_FIXED, fd, 4096*2);
    if(tmp==MAP_FAILED)
        errExit("mmap");
    tmp = mmap(addr+4096, 4096, PROT_READ, MAP_SHARED|MAP_FIXED, fd, 4096);
    if(tmp==MAP_FAILED)
        errExit("mmap");
    tmp = mmap(addr+4096*2, 4096, PROT_READ, MAP_SHARED|MAP_FIXED, fd, 0);
    if(tmp==MAP_FAILED)
        errExit("mmap");

    memcpy(&buf, addr, 4);
    printf("Byte 0 after non linear mapping: %s \n", buf);
    memcpy(&buf, (addr+4000), 4);
    printf("Byte 4000 after non linear mapping: %s \n", buf);
    memcpy(&buf, (addr+8000), 4);
    printf("Byte 8000 after non linear mapping: %s \n", buf);
    memcpy(&buf, (addr+12000), 4);
    printf("Byte 12000 after non linear mapping: %s \n", buf);

    if(close(fd)==-1)
        errExit("close");
    

    


    exit(EXIT_SUCCESS);
}
