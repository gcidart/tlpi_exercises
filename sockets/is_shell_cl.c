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
#include <arpa/inet.h>
#include <netdb.h>
#include "tlpi_hdr.h"
#include "inet_sockets.h"       /* Declarations of inet*() socket functions */

#define PORT_NUM "50000"        /* Port number for server */
#define BUF_SIZE 100

int
main(int argc, char *argv[])
{
    int sfd;
    ssize_t numRead;
    char buf[BUF_SIZE];

    if (argc != 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s host 'some-shell-command'\n", argv[0]);

    sfd = inetConnect(argv[1], PORT_NUM, SOCK_STREAM);
    if (sfd == -1)
        errExit("inetConnect");

    switch (fork()) {
    case -1:
        errExit("fork");

    case 0:             /* Child: read server's response, echo on stdout */
        for (;;) {
            numRead = read(sfd, buf, BUF_SIZE);
            if (numRead <= 0)                   /* Exit on EOF or error */
                break;
            printf("%.*s", (int) numRead, buf);
        }
        exit(EXIT_SUCCESS);

    default:            /* Parent: write contents of stdin to socket */
        //for (;;) {
            //numRead = read(STDIN_FILENO, buf, BUF_SIZE);
            //if (numRead <= 0)                   /* Exit loop on EOF or error */
              //  break;
            //if (write(sfd, argv[2] , strlen(argv[2])) != numRead)
            if (write(sfd, argv[2] , strlen(argv[2])) <=0 )
                fatal("write() failed");
        //}

        /* Close writing channel, so server sees EOF */

        if (shutdown(sfd, SHUT_WR) == -1)
            errExit("shutdown");
        exit(EXIT_SUCCESS);
    }
}
