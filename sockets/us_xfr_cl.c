/*
 *Rewrite the programs in Listing 57-3 (us_xfr_sv.c) and Listing 57-4 (us_xfr_cl.c) to use the Linux-specific abstract socket namespace (Section 57.6).
 *
 *Write a UNIX domain sockets library with an API similar to the Internet domain sockets library shown in Section 59.12.
 *Rewrite the programs in Listing 57-3 (us_xfr_sv.c, on page 1168) and Listing 57-4 (us_xfr_cl.c, on page 1169) to use this library.
 *
 *
 * $ sockets/us_xfr_sv > test/b &
 * [1] 9699
 * $ sockets/us_xfr_cl < /etc/services
 * $ kill %1
 * $ 
 * [1]+  Terminated              sockets/us_xfr_sv > test/b
 * $ diff test/b /etc/services
 * $ 
 * 
 */
#include "unix_sockets.h"

#define SV_SOCK_PATH "us_xfr"

#define BUF_SIZE 100


#define BACKLOG 5

int
main(int argc, char *argv[])
{
    int sfd;
    ssize_t numRead;
    char buf[BUF_SIZE];
    char *sv_addr = malloc(strlen(SV_SOCK_PATH));


    sfd = unixConnect(sv_addr, SOCK_STREAM);
    if (sfd == -1)
        errExit("unixConnect");


    /* Copy stdin to socket */

    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0)
        if (write(sfd, buf, numRead) != numRead)
            fatal("partial/failed write");

    if (numRead == -1)
        errExit("read");

    exit(EXIT_SUCCESS);         /* Closes our socket; server sees EOF */
}
