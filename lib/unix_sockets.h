/* unix_sockets.h

   Header file for unix_sockets.c.
*/
#ifndef UNIX_SOCKETS_H
#define UNIX_SOCKETS_H          /* Prevent accidental double inclusion */

#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include "tlpi_hdr.h"



int unixConnect(const char *host, int type);

int unixListen(const char *host, int backlog);

int unixBind(const char *host, int type);

#endif
