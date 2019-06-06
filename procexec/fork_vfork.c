/*
 *
 *Write a program to see how fast the fork() and vfork() system calls are on your system.
 *Each child process should immediately exit, and the parent should wait() on each child before creating the next. 
 *Compare the relative differences for these two system calls with those of Table 28-3.
 *The shell built-in command time can be used to measure the execution time of a program.
 * $ time procexec/fork_vfork fork
 * 
 * real	0m4.126s
 * user	0m1.890s
 * sys	0m1.898s
 * $ time procexec/fork_vfork vfork
 * 
 * real	0m1.095s
 * user	0m0.340s
 * sys	0m0.634s
 * 
 */

#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tlpi_hdr.h"



int
main(int argc, char *argv[])
{
    pid_t childPid;

    if (argc !=2 || ( strcmp(argv[1], "--help") == 0))
        usageErr("%s fork|vfork \n", argv[0]);
    for(int i =0; i < 100000; i++)
    {
        if(strcmp(argv[1], "fork")==0)
            childPid = fork();
        else
            childPid = vfork();
        if(childPid == -1)
            errExit("fork");
        else if(childPid == 0)
            _exit(EXIT_SUCCESS);
        else
        {
            wait(NULL);
        }
    }
            

    exit(EXIT_SUCCESS);
    
}

