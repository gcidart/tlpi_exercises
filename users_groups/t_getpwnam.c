/*
 * Implement getpwnam() using setpwent(), getpwent(), and endpwent()
 *
 *
 * This implementation does not return a pointer to static area, 
 * which maybe the case in some Linux implementations.
 *
 * http://man7.org/linux/man-pages/man3/getpwuid.3.html
*/
#include <pwd.h>
#include "tlpi_hdr.h"

struct passwd *
t_getpwnam(const char *name);


int
main(int argc, char *argv[])
{
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s username \n", argv[0]);

    struct passwd *pw = t_getpwnam(argv[1]);
    if(!pw)
	printf("User not found \n");
    else
        printf("%-8s %5ld\n", pw->pw_name, (long) pw->pw_uid);
    exit(EXIT_SUCCESS);
}

struct passwd *
t_getpwnam(const char *name)
{
    struct passwd *pwd;
    struct passwd *ret =  (struct passwd *)malloc(sysconf(_SC_GETPW_R_SIZE_MAX)) ;
    int found = 0;

    while ((pwd = getpwent()) != NULL)
    {
	if(strcmp(pwd->pw_name, name)==0)
	{
		ret->pw_name = strdup(pwd->pw_name);
		ret->pw_passwd = strdup(pwd->pw_passwd);
		ret->pw_gecos = strdup(pwd->pw_gecos);
		ret->pw_dir = strdup(pwd->pw_dir);
		ret->pw_shell = strdup(pwd->pw_shell);
		ret->pw_uid = pwd->pw_uid;
		ret->pw_gid = pwd->pw_gid;
		found = 1;
	}
    }
    endpwent();
    return (found==1)?ret:NULL;
    
}

