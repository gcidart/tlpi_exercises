/*
 *Implement initgroups() using setgroups() and library functions for retrieving information from the password and group files (Section 8.4).
 * Remember that a process must be privileged in order to be able to call setgroups()
 *
*/

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <grp.h>
#include <limits.h>
#include "tlpi_hdr.h"

int 
t_initgroups(const char *user, gid_t group);

int
main(int argc, char *argv[])
{
    
    gid_t glist[100]; 
    int num_grps;
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s username \n", argv[0]);

    if(initgroups(argv[1],0)==-1)
    	errExit("initgroups");
    num_grps = getgroups(100, glist);
    for(int i =0; i < num_grps; i++)
        printf("Group id: %ld\n", (long) glist[i]);

    return 0;
}



int 
t_initgroups(const char *user, gid_t group)
{
    struct passwd *pwd;
    uid_t u;
    gid_t grouplist[NGROUPS_MAX+1];
    pwd = getpwnam(user);
    if(pwd==NULL)
        return -1;
    u = pwd->pw_uid;
    seteuid(u);
    if(setgroups(NGROUPS_MAX, grouplist)==-1)
    	return -1;
    if (setuid(getuid()) == -1)
    	errExit("setuid");
    return(EXIT_SUCCESS);
}




