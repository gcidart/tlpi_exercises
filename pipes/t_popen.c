/*
 *Implement popen() and pclose(). Although these functions are simplified by not requiring the signal handling employed in the implementation of system() (Section 27.7),
 * you will need to be careful to correctly bind the pipe ends to file streams in each process, and to ensure that all unused descriptors referring to the pipe ends are closed.
 *Since children created by multiple calls to popen() may be running at one time,
 * you will need to maintain a data structure that associates the file stream pointers allocated by popen() with the corresponding child process IDs.
 *(If using an array for this purpose, the value returned by the fileno() function, which obtains the file descriptor corresponding to a file stream, can be used to index the array.)
 *Obtaining the correct process ID from this structure will allow pclose() to select the child upon which to wait.
 *This structure will also assist with the SUSv3 requirement that any still-open file streams created by earlier calls to popen() must be closed in the new child process.  
 * 
 * $ ./pipes/t_popen 
 * ++  PID TTY          TIME CMD
 * ++ 4255 pts/0    00:00:01 bash
 * ++ 6916 pts/0    00:00:00 t_popen
 * ++ 6917 pts/0    00:00:00 sh
 * ++ 6918 pts/0    00:00:00 ps
 *       2       4      25
 * 
*/

#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 100
static int arr[10000];

static FILE *
t_popen(const char *command, const char *type)
{
    int pfd[2], child_pid;                                     /* Pipe file descriptors */
    FILE * stream;

    if (pipe(pfd) == -1)                            /* Create pipe */
        errExit("pipe");
    if(strcmp(type, "r")==0)
        {   
            char mode = 'r';
            stream = fdopen(pfd[0], &mode);
        }
        else
        {   
            char mode = 'w';
            stream = fdopen(pfd[1], &mode);
        }
    

    switch (child_pid = fork()) {
    case -1:
        errExit("fork");

    case 0: 
        if(strcmp(type, "r")==0)
        {   
            if (close(pfd[0]) == -1)                    /* Read end is unused */
                errExit("close");
            if (pfd[1] != STDOUT_FILENO) {              /* Defensive check */
                if (dup2(pfd[1], STDOUT_FILENO) == -1)
                    errExit("dup2 ");
                if (close(pfd[1]) == -1)
                    errExit("close ");
            }

        }
        else
        {
            if (close(pfd[1]) == -1)                    /* Write end is unused */
                errExit("close");
            if (pfd[0] != STDIN_FILENO) {              /* Defensive check */
                if (dup2(pfd[0], STDIN_FILENO) == -1)
                    errExit("dup2 ");
                if (close(pfd[0]) == -1)
                    errExit("close ");

            }
        }

        execl("/bin/sh", "sh", "-c", command, (char *) NULL);
        _exit(127);                     /* We could not exec the shell */
    default:            
        arr[fileno(stream)] = child_pid;            
        if(strcmp(type, "r")==0)
        {   
            if (close(pfd[1]) == -1)                    /* Write end is unused */
                errExit("close");
        }
        else
        {   
            if (close(pfd[0]) == -1)                    /* Read end is unused */
                errExit("close");
        }

        return stream;
    }
}

    

static int 
t_pclose(FILE *stream){
    int status;
    int childPid = arr[fileno(stream)];
    fclose(stream);
    while (waitpid(childPid, &status, 0) == -1) {
               if (errno != EINTR) {       /* Error other than EINTR */
                   status = -1;
                   break;                  /* So exit loop */
               }
           }
    return status;
}

int
main(int argc, char *argv[])
{
    char res[BUF_SIZE];
    FILE *fp = t_popen("ps", "r");
    if (fp == NULL) {
            errExit("t_popen");
    }
    while (fgets(res, BUF_SIZE, fp) != NULL) {
            printf("++%s", res);
    }


    int status = t_pclose(fp);
    if(status==-1)
        errExit("pclose");
    fp = t_popen("wc", "w");
    char *str = "First line \n Second line\n";
    fputs(str,fp);
    status = t_pclose(fp);
    if(status==-1)
        errExit("pclose");
    exit(EXIT_SUCCESS);
}
