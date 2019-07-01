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
#include <sys/wait.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/time.h>
#include <sys/select.h>
#include "inet_sockets.h"       /* Declarations of inet*() socket functions */
#include "tlpi_hdr.h"

static void             /* SIGCHLD handler to reap dead child processes */
grimReaper(int sig)
{
    int savedErrno;             /* Save 'errno' in case changed here */

    savedErrno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        continue;
    errno = savedErrno;
}

/* Handle a client request: copy socket input back to socket */

static void
handleRequest(int cfd)
{
    char buf[BUF_SIZE],  service[NI_MAXSERV], host[NI_MAXHOST], rcookie[COOKIE_LEN];
    struct sockaddr_storage addr ;
    socklen_t as;
    ssize_t numRead;
    int pfd;
    /* Get socket number for client's priority socket */
    if(read(cfd, service, NI_MAXSERV) <=0)
    {   
        syslog(LOG_ERR, "read() failed to get port number of priority port:%s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    as = sizeof(struct sockaddr_storage);
    if(getpeername(cfd, (struct sockaddr *) & addr, &as)==-1)
    {   
        syslog(LOG_ERR, "getpeername() failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (getnameinfo((struct sockaddr *) &addr, as, host, NI_MAXHOST,
                    NULL, NI_MAXSERV, NI_NUMERICSERV) != 0)
    {   
        syslog(LOG_ERR, "getnameinfo() failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* Connect to client's priority socket */
    pfd = inetConnect(host, service, SOCK_STREAM);
    if(pfd == -1)
    {   
        syslog(LOG_ERR, "inetConnect() failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* Read cookie from client's normal socket 
        and sendback it back */
    numRead = read(pfd, rcookie, COOKIE_LEN);
    if(numRead > 0)
    {
        if(write(pfd, rcookie, COOKIE_LEN)!=numRead)
        {   
            syslog(LOG_ERR, "cookie write() failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else
    {   
        syslog(LOG_ERR, "cookie read() failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    for(int i =0; i<=50; i++)
    {
        snprintf(buf, BUF_SIZE, "%d", i);
        if(i%5==0)
        {
            /*First write to priorty socket would fail if cookie exchange failed in previous step */
            if(write(pfd, buf, BUF_SIZE)==-1) 
                break;
        }
        else
            write(cfd, buf, BUF_SIZE);
    }
    exit(EXIT_SUCCESS);

}

int
main(int argc, char *argv[])
{
    int sfd, cfd ;
    struct sigaction sa;

        /* Establish SIGCHLD handler to reap terminated child processes */

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = grimReaper;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Error from sigaction(): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }



    sfd = inetListen(PORT_NUM, 10, NULL);
    if(sfd==-1)
        errExit("inetListen");

    for (;;) {
        cfd = accept(sfd, NULL, NULL);  /* Wait for connection */
        if (cfd == -1) {
            syslog(LOG_ERR, "Failure in accept(): %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        /* Handle each client request in a new child process */

        switch (fork()) {
        case -1:
            syslog(LOG_ERR, "Can't create child (%s)", strerror(errno));
            close(cfd);                 /* Give up on this client */
            break;                      /* May be temporary; try next client */

        case 0:                         /* Child */
            handleRequest(cfd);
            _exit(EXIT_SUCCESS);

        default:                        /* Parent */
            close(cfd);                 /* Unneeded copy of connected socket */
            break;                      /* Loop to accept next connection */
        }
    }
}


