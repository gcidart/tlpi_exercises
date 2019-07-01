/*
 * Implement a replacement for sendfile() using read(), write(), and lseek().
 * 
 * 
 * $ sockets/t_sendfile > test/sf_test.txt
 * $ diff /etc/services test/sf_test.txt 
 * 1c1
 * < # Network services, Internet style
 * ---
 * > work services, Internet style
 * 
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "tlpi_hdr.h"
#include "unix_sockets.h"

#define SOCK_PATH "/tmp/sp_sf"
#define BACKLOG 5

#define BUF_SIZE 4096

static ssize_t 
t_sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
    char buf[BUF_SIZE];
    off_t oo;
    size_t numRead, numXfr;

    if(offset!=NULL)
    {
        oo = lseek(in_fd, 0, SEEK_CUR);
        if(oo==-1)
            return -1;
        if(lseek(in_fd, *offset, SEEK_SET)==-1)
            return -1;
    }
    while (count>0)
    {
        numRead = read(in_fd, buf,min(BUF_SIZE, count));
        if(numRead==-1)
            return -1;
        if(numRead==0)
            break;
        if(write(out_fd, buf, numRead)!= numRead)
            return -1;

        count-= numRead;
        numXfr+= numRead;
    }
    if(offset!=NULL)
    {
        *offset = lseek(in_fd, 0 , SEEK_CUR);
        if(*offset < 0)
            return -1;
        if(lseek(in_fd, oo, SEEK_SET)==-1)
            return -1;
    }
        
    return numXfr;
}


int
main(int argc, char *argv[])
{
    int sfd, cfd, fd;
    ssize_t numRead;
    char buf[BUF_SIZE];
    char *sv_addr = malloc(strlen(SOCK_PATH));
    off_t of = 5;

    sfd = unixListen(sv_addr, BACKLOG);
    if (sfd == -1)
        errExit("unixListen");

    switch (fork()) {
    case -1:
        errExit("fork");
    case 0:
        cfd = unixConnect(sv_addr, SOCK_STREAM);
        if(cfd==-1)
            errExit("unixConnect");
        for (;;) {
            numRead = read(cfd, buf, BUF_SIZE);
            if (numRead <= 0)                   /* Exit on EOF or error */
                break;
            printf("%.*s", (int) numRead, buf);
        }
        exit(EXIT_SUCCESS);
    default:
        cfd = accept(sfd, NULL, NULL);
        if(cfd==-1)
            errExit("accept");
        fd = open("/etc/services", O_RDONLY);
        if(fd==-1)
            errExit("open");
        if(t_sendfile(cfd, fd, &of, 2000000)==-1)
            errExit("t_sendfile");
        exit(EXIT_SUCCESS);
    }
}
        
        
    
        
