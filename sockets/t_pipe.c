/*
 *Implement pipe() in terms of socketpair(). Use shutdown() to ensure that the resulting pipe is unidirectional.
 *
 * $ ./sockets/t_pipe hello
 * hello
 *
 */

#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "tlpi_hdr.h"

static int
t_pipe(int * pipefd)
{
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd)==-1)
        return -1;
    if(shutdown(pipefd[0], SHUT_WR)==-1)
        return -1;
    if(shutdown(pipefd[1], SHUT_RD)==-1)
        return -1;
    return 0;
}

int
main(int argc, char *argv[])
{
    int pipefd[2];
    pid_t cpid;
    char buf;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <string>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (t_pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    cpid = fork();
    if (cpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid == 0) {    /* Child reads from pipe */
        close(pipefd[1]);          /* Close unused write end */

        while (read(pipefd[0], &buf, 1) > 0)
            write(STDOUT_FILENO, &buf, 1);

        write(STDOUT_FILENO, "\n", 1);
        close(pipefd[0]);
        _exit(EXIT_SUCCESS);

    } else {            /* Parent writes argv[1] to pipe */
        close(pipefd[0]);          /* Close unused read end */
        write(pipefd[1], argv[1], strlen(argv[1]));
        close(pipefd[1]);          /* Reader will see EOF */
        wait(NULL);                /* Wait for child */
        exit(EXIT_SUCCESS);
    }
}
    
