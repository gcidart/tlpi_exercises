/*
 *Section 61.13.1 noted that an alternative to out-of-band data would be to create two socket connections between the client and server: one for normal data and one for priority data.
 *Write client and server programs that implement this framework. Here are a few hints:
 *
 *• The server needs some way of knowing which two sockets belong to the same client.
 *One way to do this is to have the client first create a listening socket using an ephemeral port (i.e., binding to port 0).
 *After obtaining the ephemeral port number of its listening socket (using getsockname()), the client connects its “normal” socket to the server’s listening socket
 * and sends a message containing the port number of the client’s listening socket.
 *The client then waits for the server to use the client’s listening socket to make a connection in the opposite direction for the “priority” socket.
 *(The server can obtain the client’s IP address during the accept() of the normal connection.)
 * 
 *• Implement some type of security mechanism to prevent a rogue process from trying to connect to the client’s listening socket.
 *To do this, the client could send a cookie (i.e., some type of unique message) to the server using the normal socket.
 *The server would then return this cookie via the priority socket so that the client could verify it.
 * 
 *• In order to experiment with transmitting normal and priority data from the client to the server,
 * you will need to code the server to multiplex the input from the two sockets using select() or poll() (described in Section 63.2).
 * 
 * 
 * $ ./sockets/oo_sv &
 * [1] 14195
 * $ ./sockets/oo_cl localhost 
 * Priority Socket: 0
 * Normal Socket: 1
 * Priority Socket: 5
 * Normal Socket: 2
 * Priority Socket: 10
 * Normal Socket: 3
 * Normal Socket: 4
 * Normal Socket: 6
 * Normal Socket: 7
 * Normal Socket: 8
 * Normal Socket: 9
 * Normal Socket: 11
 * Priority Socket: 15
 * Normal Socket: 12
 * Normal Socket: 13
 * Normal Socket: 14
 * Normal Socket: 16
 * Normal Socket: 17
 * Normal Socket: 18
 * Normal Socket: 19
 * Priority Socket: 20
 * Priority Socket: 25
 * Normal Socket: 21
 * Priority Socket: 30
 * Normal Socket: 22
 * Priority Socket: 35
 * Normal Socket: 23
 * Priority Socket: 40
 * Normal Socket: 24
 * Priority Socket: 45
 * Normal Socket: 26
 * Priority Socket: 50
 * Normal Socket: 27
 * Normal Socket: 28
 * Normal Socket: 29
 * Normal Socket: 31
 * Normal Socket: 32
 * Normal Socket: 33
 * Normal Socket: 34
 * Normal Socket: 36
 * Normal Socket: 37
 * Normal Socket: 38
 * Normal Socket: 39
 * Normal Socket: 41
 * Normal Socket: 42
 * Normal Socket: 43
 * Normal Socket: 44
 * Normal Socket: 46
 * Normal Socket: 47
 * Normal Socket: 48
 * Normal Socket: 49
 * 
 * 
 *
 */

#define PORT_NUM "50000"
#define COOKIE_LEN 20
#define BUF_SIZE 20

#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/time.h>
#include <sys/select.h>
#include "inet_sockets.h"       /* Declarations of inet*() socket functions */
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int nfd, ofd, pfd, numWrite, numReadp, numReadn, nfds, ready;
    struct sockaddr *addr ;
    socklen_t as ;
    char service[NI_MAXSERV], buf[BUF_SIZE];
    char cookie[COOKIE_LEN], rcookie[COOKIE_LEN];
    fd_set readfds;
    
    

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s host\n", argv[0]);

    /*Priority Socket */
    ofd = inetListen("0", 0, NULL);
    if(ofd==-1)
        errExit("inetListen");
    
    /* Connect to server */
    nfd = inetConnect(argv[1], PORT_NUM, SOCK_STREAM);
    if(nfd==-1)
        errExit("inetConnect");

    addr = malloc(sizeof(struct sockaddr));
    as = sizeof(struct sockaddr);

    if(getsockname(ofd, addr, &as)==-1)
        errExit("getsockname");
    addr = realloc(addr, as);
    if(getsockname(ofd, addr, &as)==-1)
        errExit("getsockname");

    /* Get priority socket number */
    if (getnameinfo(addr, as, NULL, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV) != 0)
        errExit("getnameinfo");


    /*Send Priority Socket number to server  */
    if(write(nfd, service, NI_MAXSERV)==-1)
        errExit("write:Out of band port number");
    
    /* pid is chosen as the cookie for demonstration */
    snprintf(cookie, COOKIE_LEN, "%ld", (long)getpid());


    /* For loop to get back the cookie and ignore other connections */
    for(;;)
    {
        pfd = accept(ofd, NULL, NULL);
        if(pfd==-1)
        {
            syslog(LOG_ERR, "accept() failed: %s", strerror(errno));
            continue;
        }
        numWrite = write(pfd, cookie, COOKIE_LEN);
        if(numWrite < 0)
        {
            syslog(LOG_ERR, "write() failed: %s", strerror(errno));
            close(pfd);
            continue;
        }
        if(read(pfd, rcookie, COOKIE_LEN)!=numWrite)
        {
            syslog(LOG_ERR, "cookie readback");
            close(pfd);
            continue;
        }
        if(strcmp(cookie, rcookie)==0)
            break;
        else
        {
            syslog(LOG_ERR, "cookie did not match");
            close(pfd);
            continue;
        }
    }

    nfds = max(nfd, pfd)+1;
    FD_ZERO(&readfds);
    for(;;)
    {

        if(FD_ISSET(pfd, &readfds))
        {
            numReadp = read(pfd, buf, BUF_SIZE);
            if (numReadp > 0)                   
                printf("Priority Socket: %.*s\n", (int) numReadp, buf);
        }
        if(FD_ISSET(nfd, &readfds))
        {
            numReadn = read(nfd, buf, BUF_SIZE);
            if (numReadn > 0)                   
                printf("Normal Socket: %.*s\n", (int) numReadn, buf);
        }
        if((numReadp <=0) && (numReadn<=0))  /* EOF reached on both sockets */
            break;
        FD_ZERO(&readfds);
        FD_SET(pfd, &readfds);
        FD_SET(nfd, &readfds);
        ready = select(nfds, &readfds, NULL, NULL, NULL);
                                        /* Ignore exceptional events */
        if (ready == -1)
            errExit("select");
    }
    exit(EXIT_SUCCESS);
}
        
        

   


        





