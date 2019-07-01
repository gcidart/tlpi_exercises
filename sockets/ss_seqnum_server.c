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
    int sfd, cfd;
    int seqLenReq;
    int seqNumResp=0;
    struct sockaddr_un saddr;

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);      /* Create server socket */
    if (sfd == -1)
        errExit("socket");

    if (strlen(SERVER_PATH) > sizeof(saddr.sun_path) - 1)
        fatal("Server socket path too long: %s", SERVER_PATH);

    if (remove(SERVER_PATH) == -1 && errno != ENOENT)
        errExit("remove-%s", SERVER_PATH);

    /* Construct server address, and bind the address */

    memset(&saddr, 0, sizeof(struct sockaddr_un));
    saddr.sun_family = AF_UNIX;
    strncpy(saddr.sun_path, SERVER_PATH, sizeof(saddr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *) &saddr, sizeof(struct sockaddr_un)) == -1)
        errExit("bind");

    if (listen(sfd, BACKLOG) == -1)
        errExit("listen");

    for (;;) {          /* Handle client connections iteratively */

        /* Accept a connection. The connection is returned on a new
           socket, 'cfd'; the listening socket ('sfd') remains open
           and can be used to accept further connections. */

        cfd = accept(sfd, NULL,NULL);
        if (cfd == -1)
            errExit("accept");

        if(read(cfd, &seqLenReq, sizeof(int))==-1)
        {
            fprintf(stderr, "Error reading request; discarding\n");
            continue;                   /* error */
        }

        if(write(cfd, &seqNumResp, sizeof(int))==-1)
        {
            fprintf(stderr, "Error writing; discarding\n");
            continue;                   /* error */
        }
        if (close(cfd) == -1)
            errMsg("close");
        
        seqNumResp += seqLenReq;
    }

}



    
