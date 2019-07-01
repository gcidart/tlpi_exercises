/* unix_sockets.c

   A package of useful routines for Internet domain sockets.
*/
#define _BSD_SOURCE             /* To get NI_MAXHOST and NI_MAXSERV
                                   definitions from <netdb.h> */
#include <sys/socket.h>
#include <netinet/in.h>
#include "unix_sockets.h"       /* Declares functions defined here */
#include "tlpi_hdr.h"

/* The following arguments are common to several of the routines
   below:

        'host':         Server address
        'type':         either SOCK_STREAM or SOCK_DGRAM
*/

/* Create socket and connect it to the address specified by
  'host' + 'service'/'type'. Return socket descriptor on success,
  or -1 on error */

int
unixConnect(const char *host,  int type)
{
    struct sockaddr_un addr;
    int sfd;

    sfd = socket(AF_UNIX, type, 0);      /* Create  socket */
    if (sfd == -1)
        return -1;

    /* Construct server address, and make the connection */

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(&addr.sun_path[1], host, sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr *) &addr,
                sizeof(struct sockaddr_un)) == -1)
        return -1;
    return sfd;
}

/* Create an UNIX domain socket and bind it to 'host'
   If 'doListen' is TRUE, then make this a listening socket (by
   calling listen() with 'backlog').
   Return the socket descriptor on success, or -1 on error. */

static int              /* Public interfaces: unixBind() and unixListen() */
unixPassiveSocket(const char *host, int type, Boolean doListen, int backlog)
{
    struct sockaddr_un addr;
    int sfd;

    sfd = socket(AF_UNIX, type, 0);      /* Create  socket */
    if (sfd == -1)
        return -1;

    /* Construct server address, and make the connection */

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(&addr.sun_path[1], host, sizeof(addr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *) &addr,
                sizeof(struct sockaddr_un)) == -1)
        return -1;


    if ( doListen) {
        if (listen(sfd, backlog) == -1) {
            return -1;
        }
    }


    return sfd;
}

/* Create stream socket, bound 'host'. 
   Make the socket a listening socket, with the specified
  'backlog'. Return socket descriptor on success, or -1 on error. */

int
unixListen(const char *host, int backlog)
{
    return unixPassiveSocket(host, SOCK_STREAM, TRUE, backlog);
}

/* Create socket bound to 'host'. Return socket descriptor on success, or -1 on error. */

int
unixBind(const char *host, int type)
{
    return unixPassiveSocket(host, type, FALSE, 0);
}

