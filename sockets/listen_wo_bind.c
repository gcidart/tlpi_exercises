/*
 *Write a program that uses getsockname() to show that, if we call listen() on a TCP socket without first calling bind(), the socket is assigned an ephemeral port number.
 * 
 * 
 * $ ./sockets/listen_wo_bind &
 * [5] 4880
 * $ getsockname(sfd): (0.0.0.0, 42919)
 * 
 * $ telnet localhost 42919 &
 * [6] 9901
 * $ Trying 127.0.0.1...
 * Connected to localhost.
 * Escape character is '^]'.
 * 
 * 
 * [6]+  Stopped                 telnet localhost 42919
 * $ netstat | grep 42919
 * tcp        0      0 localhost:56590         localhost:42919         ESTABLISHED
 * tcp        0      0 localhost:42919         localhost:56590         ESTABLISHED
 * $ ./sockets/listen_wo_bind &
 * [8] 9995
 * $ getsockname(sfd): (0.0.0.0, 49953)
 * 
 * $ telnet localhost 49953 &
 * [9] 9996
 * $ Trying 127.0.0.1...
 * Connected to localhost.
 * Escape character is '^]'.
 * 
 * 
 * [9]+  Stopped                 telnet localhost 49953
 * $ netstat | grep 49953
 * tcp        0      0 localhost:59738         localhost:49953         ESTABLISHED
 * tcp        0      0 localhost:49953         localhost:59738         ESTABLISHED
 * 
 */

#include <sys/socket.h>
#include <netdb.h>
#include "tlpi_hdr.h"

#define IS_ADDR_STR_LEN 20

static char *
inetAddressStr(const struct sockaddr *addr, socklen_t addrlen,
               char *addrStr, int addrStrLen)
{
    char host[NI_MAXHOST], service[NI_MAXSERV];

    if (getnameinfo(addr, addrlen, host, NI_MAXHOST,
                    service, NI_MAXSERV, NI_NUMERICSERV) == 0)
        snprintf(addrStr, addrStrLen, "(%s, %s)", host, service);
    else
        snprintf(addrStr, addrStrLen, "(?UNKNOWN?)");

    return addrStr;
}

int
main(int argc, char *argv[])
{
    int sfd;
    char addrStr[IS_ADDR_STR_LEN];
    struct sockaddr *addr = malloc(sizeof(struct sockaddr));
    socklen_t as = sizeof(struct sockaddr);
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd==-1)
        errExit("socket");
    if(listen(sfd, 10) ==-1)
        errExit("listen");
    if(getsockname(sfd, addr, &as)==-1)
        errExit("getsockname");
    addr = realloc(addr, as);
    if(getsockname(sfd, addr, &as)==-1)
        errExit("getsockname");
    printf("getsockname(sfd): %s\n",
            inetAddressStr(addr, as, addrStr, IS_ADDR_STR_LEN));
    sleep(60);
}
    
    
