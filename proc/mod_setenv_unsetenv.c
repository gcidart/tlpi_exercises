/*
* Implement setenv() and unsetenv() using getenv(), putenv(),and, where necessary, code that directly modifies environ. 
* Your version of unsetenv() should check to see whether there are multiple definitions of an environment variable, 
* and remove them all (which is what the glibc version of unsetenv() does
*
* Examples from Textbook:
* $ ./mod_setenv_unsetenv "GREET=Guten Tag" SHELL=/bin/bash BYE=Ciao
*	Match found for BYE
*	GREET=Guten Tag
*	SHELL=/bin/bash
* $ ./mod_setenv_unsetenv ./modify_env SHELL=/bin/sh BYE=byebye
*	Match found for BYE
*	SHELL=/bin/sh
*	GREET=Hello world
* 	
* 
*/
#include <stdlib.h>
#include "tlpi_hdr.h"

extern char **environ;

int 
t_setenv(const char *name, const char *value, int overwrite);

int
t_unsetenv(const char *name);

int
main(int argc, char *argv[])
{
    int j;
    char **ep;

    clearenv();         /* Erase entire environment */

    /* Add any definitions specified on command line to environment */

    for (j = 1; j < argc; j++)
        if (putenv(argv[j]) != 0)
            errExit("putenv: %s", argv[j]);

    /* Add a definition for GREET if one does not already exist */

    if (t_setenv("GREET", "Hello world", 0) == -1)
        errExit("setenv");

    /* Remove any existing definition of BYE */

    t_unsetenv("BYE");

    /* Display current environment */

    for (ep = environ; *ep != NULL; ep++)
        puts(*ep);

    exit(EXIT_SUCCESS);
}


int 
t_setenv(const char *name, const char *value, int overwrite)
{
    char *nes;
    if(getenv(name) && !overwrite)
    {
        return 0;
    }
    t_unsetenv(name);
    if((nes = malloc(strlen(name) + strlen(value) +2))==NULL)
	return -1;
    strcpy(nes, name);
    strcat(nes, "=");
    strcat(nes, value);
    if(putenv(nes)==0)
	return 0;
    else
	return -1;
} 

int
t_unsetenv(const char *name)
{
    char **ev, **ev1;
    
    for(ev=environ; *ev!= NULL; ev++)
    {
	if(!strncmp(*ev, name, strlen(name)) &&((strchr(*ev, '=')-*ev) ==(strlen(name))))
	{
	    printf("Match found for %s\n", name);
	    for(ev1=ev; *ev1!=NULL; ev1++)
	    {
                *ev1 = *(ev1+1);
	    }
	    *ev1 = NULL;
	    break;
	}
    }
    return 0;
}


