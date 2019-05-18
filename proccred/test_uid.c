/*
 *
 * Assume in each of the following cases that the initial set of process user IDs is real=1000 effective=0 saved=0 file-system=0.
 * What would be the state of the user IDs after the following calls?
 *
 *a) setuid(2000);
 *
 *b) setreuid(–1, 2000);
 *
 *c) seteuid(2000);
 *
 *d) setfsuid(2000);
 *
 *e) setresuid(–1, 2000, 3000);
 *
 *
 * x@-VirtualBox:~/tlpi_ex/proccred$ sudo su
 *root@-VirtualBox:/home/x/tlpi_ex/proccred# chown root test_uid
 *root@-VirtualBox:/home/x/tlpi_ex/proccred# chmod u+s test_uid
 *root@-VirtualBox:/home/x/tlpi_ex/proccred# chmod g+s test_uid
 *root@-VirtualBox:/home/x/tlpi_ex/proccred# exit
 *exit
 *x@-VirtualBox:~/tlpi_ex/proccred$ ./test_uid 1
 *         UID              GID  
 *Real      1000     Real      1000  
 *Effective 0     Effective 1000  
 *Saved     0     Saved     1000  
 *After setuid(2000)
 *         UID              GID  
 *Real      2000     Real      1000  
 *Effective 2000     Effective 1000  
 *Saved     2000     Saved     1000  
 *x@-VirtualBox:~/tlpi_ex/proccred$ ./test_uid 2
 *         UID              GID  
 *Real      1000     Real      1000  
 *Effective 0     Effective 1000  
 *Saved     0     Saved     1000  
 *After setreuid(-1, 2000)
 *         UID              GID  
 *Real      1000     Real      1000  
 *Effective 2000     Effective 1000  
 *Saved     2000     Saved     1000  
 *x@-VirtualBox:~/tlpi_ex/proccred$ ./test_uid 3
 *         UID              GID  
 *Real      1000     Real      1000  
 *Effective 0     Effective 1000  
 *Saved     0     Saved     1000  
 *After seteuid(2000)
 *         UID              GID  
 *Real      1000     Real      1000  
 *Effective 2000     Effective 1000  
 *Saved     0     Saved     1000  
 *x@-VirtualBox:~/tlpi_ex/proccred$ ./test_uid 4
 *         UID              GID  
 *Real      1000     Real      1000  
 *Effective 0     Effective 1000  
 *Saved     0     Saved     1000  
 *After setfsuid(2000)
 *         UID              GID  
 *Real      1000     Real      1000  
 *Effective 0     Effective 1000  
 *Saved     0     Saved     1000  
 *x@-VirtualBox:~/tlpi_ex/proccred$ ./test_uid 5
 *         UID              GID  
 *Real      1000     Real      1000  
 *Effective 0     Effective 1000  
 *Saved     0     Saved     1000  
 *After setresuid(-1,2000,3000)
 *         UID              GID  
 *Real      1000     Real      1000  
 *Effective 2000     Effective 1000  
 *Saved     3000     Saved     1000
 *
*/
#include <unistd.h>
#include <sys/fsuid.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    uid_t ruid, euid, suid;
    gid_t rgid, egid, sgid;
    getresuid(&ruid, &euid, &suid);
    getresgid(&rgid, &egid, &sgid);
    printf(
        "         UID              GID  \n"
        "Real      %d     Real      %d  \n"
        "Effective %d     Effective %d  \n"
        "Saved     %d     Saved     %d  \n",
             ruid ,       rgid,
             euid ,       egid,
	     suid ,       sgid

    );
    switch (atoi(argv[1])){
	    case 1:
		   setuid(2000);
		   printf("After setuid(2000)\n");
		   break;
	    /*For both privileged and unprivileged processes, the saved set-user-ID is also set to the same value 
	     * as the (new) effective user ID if either of the following is true:
             * a) ruid is not –1 (i.e., the real user ID is being set, even to the same value it already had), or
             * b) the effective user ID is being set to a value other than the value of the real user ID prior to the call.
            */
	    case 2:
		   setreuid(-1, 2000);
		   printf("After setreuid(-1, 2000)\n");
		   break;
	    case 3:
		   seteuid(2000);
		   printf("After seteuid(2000)\n");
		   break;
	    case 4:
		   setfsuid(2000);
		   printf("After setfsuid(2000)\n");
		   break;
	    case 5:
		   setresuid(-1,2000,3000);
		   printf("After setresuid(-1,2000,3000)\n");
		   break;
            default:
		   break;
    }
    getresuid(&ruid, &euid, &suid);
    getresgid(&rgid, &egid, &sgid);
    printf(
        "         UID              GID  \n"
        "Real      %d     Real      %d  \n"
        "Effective %d     Effective %d  \n"
        "Saved     %d     Saved     %d  \n",
             ruid ,       rgid,
             euid ,       egid,
	     suid ,       sgid

    );


    return 0;
}
