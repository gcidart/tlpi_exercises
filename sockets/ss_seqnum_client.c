/*
 *Reimplement the sequence-number server and client of Section 44.8 using UNIX domain stream sockets.
 * 
 * 
 * $ sockets/ss_seqnum_server &
 * [5] 10343
 * $ ls -lF /tmp/seqnum_sv 
 * srwxr-xr-x 1 xxx xxx 0 Jun 24 13:31 /tmp/seqnum_sv=
 * $ sockets/ss_seqnum_client 2
 * 0
 * $ sockets/ss_seqnum_client 3
 * 2
 * $ sockets/ss_seqnum_client 5
 * 5
 * $ sockets/ss_seqnum_client 1
 * 10
 * $ sockets/ss_seqnum_client 2
 * 11
 * 
 * 
 * 
 */

#include <sys/un.h>
#include <sys/socket.h>
#include "tlpi_hdr.h"

#define SERVER_PATH "/tmp/seqnum_sv"
                                /* Well-known server path */
#define BACKLOG 5

int
main(int argc, char *argv[])
{
    int sfd ;
    int seqLenReq;
    int seqNumResp;
    struct sockaddr_un saddr ;

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [seq-len]\n", argv[0]);

     

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);      /* Create client socket */
    if (sfd == -1)
        errExit("socket");
    
    
    /* Construct server address, and make the connection */

    memset(&saddr, 0, sizeof(struct sockaddr_un));
    saddr.sun_family = AF_UNIX;
    strncpy(saddr.sun_path, SERVER_PATH, sizeof(saddr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr *) &saddr,
                sizeof(struct sockaddr_un)) == -1)
        errExit("connect");

    seqLenReq = atoi(argv[1]);

    if (write(sfd, &seqLenReq, sizeof(int)) == -1)
            fatal("partial/failed write");

    
    if(read(sfd, &seqNumResp, sizeof(int)) == -1)
        errExit("read");

    printf("%d\n", seqNumResp);

    exit(EXIT_SUCCESS);
}



