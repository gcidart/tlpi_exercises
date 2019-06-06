/*
 *
 * Use execve() to implement execlp(). You will need to use the stdarg(3) API to handle the variable-length argument list supplied to execlp(). 
 * You will also need to use functions in the malloc package to allocate space for the argument and environment vectors. 
 * Finally, note that an easy way of checking whether a file exists in a particular directory and is executable is simply to try execing the file. 
 * 
 * 
 * $ which echo
 * /bin/echo
 * gsiddharth@fanta-VirtualBox:~/tlpi_ex$ ./procexec/t_execlp echo
 * hello world
 */

#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include "tlpi_hdr.h"

extern char **environ;

static int 
t_execlp(const char *file, const char *arg, ...)
{
    va_list args;
    char **argVec;
    char **envVec;
    char *temp, *path, *tok, *pathname;
    int cnt = 0;
    int i=0;
    int sea = 0;
    temp = (char *) arg;
    cnt  = 1;
    va_start(args,arg);
    while(temp!=NULL)
    {
        temp = va_arg(args, char *);
        cnt++;
    }
    va_end(args);
    argVec = malloc((cnt+1)*sizeof(void *));
    if(argVec==NULL)
        return -1;
    argVec[i] = strrchr(file, '/');      /* Get basename from file */
    if (argVec[i] != NULL)
        argVec[i]++;
    else
    {
        argVec[i] = (char *) file;
        sea = 1;
    }
    va_start(args, arg);
    argVec[++i] = (char *) arg;
    while(argVec[i]!=NULL)
    {
        argVec[++i] = va_arg(args, char *);
    }
    va_end(args);
    
    for(i=1;environ[i]!=NULL;i++);
    envVec = malloc(i*sizeof(void *));
    if(envVec == NULL)
        return -1;
    for(i=0;environ[i]!=NULL; i++)
        envVec[i] = strdup(environ[i]); 
    envVec[i] = NULL;
    
    
    if(sea == 0)
    {
        execve(file, argVec, envVec);
    }
    else
    {   
        pathname = malloc(PATH_MAX*sizeof(char *));
        if(pathname == NULL)
            return -1;
        temp = getenv("PATH");
        path = strdup(temp);
        tok = strtok(path, ":");
        while(tok!=NULL)
        {
            strcpy(pathname, tok);
            strcat(pathname, "/");
            strcat(pathname, file);
            execve(pathname, argVec, envVec);
            tok = strtok(NULL, ":");
        }
        //free(pathname);
        //free(path);
    }
    /* should not reach here if execve succeeds */
    /*free(argVec);
    i--;
    while(i>=0)
        free(envVec[i]);
    free(envVec);*/
    return -1;


}
        
        
        

int
main(int argc, char *argv[])
{
   if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname\n", argv[0]);

    /* Execute the program specified in argv[1] */

    t_execlp(argv[1],  "hello world", (char *) NULL);
}

