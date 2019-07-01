/*
 *Add code to the program in Listing 60-4 (is_echo_sv.c) to place a limit on the number of simultaneously executing children.
 *
 * # ./sockets/is_echo_sv_lim &
 * [1] 25523
 * # cat test/echo.txt
 * echo this
 * # ./sockets/is_echo_cl localhost < test/echo.txt
 * echo this
 * # ./sockets/is_echo_cl localhost < test/echo.txt
 * echo this
 * # ./sockets/is_echo_cl localhost < test/echo.txt
 * echo this
 * # ./sockets/is_echo_cl localhost < test/echo.txt
 * # ./sockets/is_echo_cl localhost < test/echo.txt
 * # ./sockets/is_echo_cl localhost < test/echo.txt
 * # ./sockets/is_echo_cl localhost < test/echo.txt
 * #
 *
 *http://man7.org/tlpi/code/online/dist/sockets/is_echo_cl.c.html
 */
#include <signal.h>
#include <syslog.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "tlpi_hdr.h"
#include "inet_sockets.h"       /* Declarations of inet*() socket functions */


#define SERVICE "echo"         /* Name of TCP service */
#define BUF_SIZE 4096
#define CONN_LIMIT 3

static int fork_cnt=0;
static int reap_cnt=0;
static void            /* SIGCHLD handler to reap dead child processes */
grimReaper(int sig)
{
    int savedErrno;            /* Save 'errno' in case changed here */

    savedErrno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
        reap_cnt++;
        continue;
    }
    errno = savedErrno;
}

/* Handle a client request: copy socket input back to socket */

static void
handleRequest(int cfd)
{
    char buf[BUF_SIZE];
    ssize_t numRead;

    while ((numRead = read(cfd, buf, BUF_SIZE)) > 0) {
        if (write(cfd, buf, numRead) != numRead) {
            syslog(LOG_ERR, "write() failed: %s", strerror(errno));
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
    int lfd, cfd;              /* Listening and connected sockets */
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = grimReaper;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Error from sigaction(): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    lfd = inetListen(SERVICE, 10, NULL);
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
        if(fork_cnt-reap_cnt >= CONN_LIMIT)
            continue;
        /* Handle each client request in a new child process */

        switch (fork()) {
        case -1:
            syslog(LOG_ERR, "Can't create child (%s)", strerror(errno));
            close(cfd);                 /* Give up on this client */
            break;                      /* May be temporary; try next client */

        case 0:                         /* Child */
            close(lfd);                 /* Unneeded copy of listening socket */
            handleRequest(cfd);
            sleep(20);                  /*Sleep to test CONN_LIMIT*/
            _exit(EXIT_SUCCESS);

        default:                        /* Parent */
            fork_cnt++;
            close(cfd);                 /* Unneeded copy of connected socket */
            break;                      /* Loop to accept next connection */
        }
    }
}
