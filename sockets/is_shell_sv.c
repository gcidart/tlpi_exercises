/*
 *Write a client and a server that permit the client to execute arbitrary shell commands on the server host.
 *(If you don’t implement any security mechanism in this application,
 *you should ensure that the server is operating under a user account where it can do no damage if invoked by malicious users.)
 *The client should be executed with two command-line arguments:
 * 
 *$ ./is_shell_cl server-host 'some-shell-command'
 * 
 *After connecting to the server, the client sends the given command to the server, and then closes its writing half of the socket using shutdown(), so that the server sees end-of-file.
 *The server should handle each incoming connection in a separate child process (i.e., a concurrent design).
 *For each incoming connection, the server should read the command from the socket (until end-of-file), and then exec a shell to perform the command.
 *Here are a couple hints:
 * 
 *• See the implementation of system() in Section 27.7 for an example of how to execute a shell command.
 * 
 *• By using dup2() to duplicate the socket on standard output and standard error, the execed command will automatically write to the socket.
 *
 * $ ./sockets/is_shell_sv &
 * [1] 11396
 * $ ./sockets/is_shell_cl localhost 'uname -p'
 * $ x86_64
 * 
 * $ ./sockets/is_shell_cl localhost 'uname -o'
 * $ GNU/Linux
 * 
 * 
 */

#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tlpi_hdr.h"
#include "inet_sockets.h"       /* Declarations of inet*() socket functions */

#define PORT_NUM "50000"        /* Port number for server */
#define BUF_SIZE 4096

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
    char buf[BUF_SIZE];
    ssize_t numRead;

    if ((numRead = read(cfd, buf, BUF_SIZE)) > 0) {
        //write(STDOUT_FILENO, buf, BUF_SIZE);
        dup2(cfd, STDOUT_FILENO);
        dup2(cfd, STDERR_FILENO);
        if (system(buf) ==-1) {
            syslog(LOG_ERR, "system() failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if (numRead == -1) {
        syslog(LOG_ERR, "Error from read(): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int
main(int argc, char *argv[])
{
    int lfd, cfd;               /* Listening and connected sockets */
    struct sigaction sa;

        /* Establish SIGCHLD handler to reap terminated child processes */

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = grimReaper;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Error from sigaction(): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    lfd = inetListen(PORT_NUM, 10, NULL);
    if (lfd == -1) {
        syslog(LOG_ERR, "Could not create server socket (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (;;) {
        cfd = accept(lfd, NULL, NULL);  /* Wait for connection */
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
            close(lfd);                 /* Unneeded copy of listening socket */
            handleRequest(cfd);
            _exit(EXIT_SUCCESS);

        default:                        /* Parent */
            close(cfd);                 /* Unneeded copy of connected socket */
            break;                      /* Loop to accept next connection */
        }
    }
}

