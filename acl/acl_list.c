/* 
 * Write a program that displays the permissions from the ACL entry that corresponds to a particular user or group.
 * The program should take two command-line arguments. The first argument is either of the letters u or g, indicating whether the second argument identifies a user or group. 
 * (The functions defined in Listing 8-1, on page 159, can be used to allow the second command-line argument to be specified numerically or as a name.) 
 * If the ACL entry that corresponds to the given user or group falls into the group class, 
 * then the program should additionally display the permissions that would apply after the ACL entry has been modified by the ACL mask entry.
 *
 * On Ubuntu, followin was needed:
 * sudo apt-get install libacl1-dev
 */



#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <acl/libacl.h>
#include <sys/acl.h>
#include "tlpi_hdr.h"

uid_t          
userIdFromName(const char *name);

gid_t           
groupIdFromName(const char *name);



int
main(int argc, char *argv[])
{

    acl_t acl;
    acl_entry_t entry;
    acl_tag_t tag;
    uid_t *uidp, uid;
    gid_t *gidp, gid;
    acl_permset_t permset;
    int entryId, permVal;
    int found = 0;
    int r =1; 
    int w =1;
    int x =1;
    if (argc !=4 || strcmp(argv[1], "--help") == 0)
        usageErr("%s u|g uid|gid filename\n", argv[0]);

    if((argv[1][0]=='u') && (argv[1][1]=='\0'))
    {
	uid = userIdFromName(argv[2]);
	if(uid==-1)
	    errExit("User not found");
    }
    else if((argv[1][0]=='g') && (argv[1][1]=='\0'))
    {
	gid = groupIdFromName(argv[2]);
	if(gid==-1)
	    errExit("Group not found");
    }
    else
        usageErr("%s u|g uid|gid filename\n", argv[0]);
    
    acl = acl_get_file(argv[3], ACL_TYPE_ACCESS);
    if (acl == NULL)
        errExit("acl_get_file");

    for (entryId = ACL_FIRST_ENTRY; ; entryId = ACL_NEXT_ENTRY) {

        if (acl_get_entry(acl, entryId, &entry) != 1)
            break;                      /* Exit on error or no more entries */

        /* Retrieve tag type */

        if (acl_get_tag_type(entry, &tag) == -1)
            errExit("acl_get_tag_type");

	 /*printf("%-12s\n", (tag == ACL_USER_OBJ) ?  "user_obj" :
                        (tag == ACL_USER) ?      "user" :
                        (tag == ACL_GROUP_OBJ) ? "group_obj" :
                        (tag == ACL_GROUP) ?     "group" :
                        (tag == ACL_MASK) ?      "mask" :
                        (tag == ACL_OTHER) ?     "other" : "???");*/

	if ((argv[1][0]=='u') && (tag == ACL_USER)) {
            uidp = acl_get_qualifier(entry);
	    //printf("%d\n", *uidp);
            if (uidp == NULL)
                errExit("acl_get_qualifier");
	    if(uid==*uidp)
		found = 1;
	    if (acl_free(uidp) == -1)
                errExit("acl_free");
	}
	else if ((argv[1][0]=='g') && (tag == ACL_GROUP)) {
            gidp = acl_get_qualifier(entry);
            if (gidp == NULL)
                errExit("acl_get_qualifier");
	    if(gid==*gidp)
		found = 1;
	    if (acl_free(uidp) == -1)
                errExit("acl_free");
	}
	else if(tag==ACL_MASK)
	{
 	    if (acl_get_permset(entry, &permset) == -1)
                errExit("acl_get_permset");

            permVal = acl_get_perm(permset, ACL_READ);
            if (permVal == -1)
                errExit("acl_get_perm - ACL_READ");
	    if (permVal == 0)
		r = 0;
            permVal = acl_get_perm(permset, ACL_WRITE);
            if (permVal == -1)
                errExit("acl_get_perm - ACL_WRITE");
	    if (permVal == 0)
		w = 0;
            permVal = acl_get_perm(permset, ACL_EXECUTE);
            if (permVal == -1)
                errExit("acl_get_perm - ACL_EXECUTE");
	    if (permVal == 0)
		x = 0;
	}


	if(found){
	    if (acl_get_permset(entry, &permset) == -1)
                errExit("acl_get_permset");
	    printf("User/Group permission:");
            permVal = acl_get_perm(permset, ACL_READ);
            if (permVal == -1)
                errExit("acl_get_perm - ACL_READ");
	    if (permVal == 0)
		r = 0;
            printf("%c", (permVal == 1) ? 'r' : '-');
            permVal = acl_get_perm(permset, ACL_WRITE);
            if (permVal == -1)
                errExit("acl_get_perm - ACL_WRITE");
	    if (permVal == 0)
		w = 0;
            printf("%c", (permVal == 1) ? 'w' : '-');
            permVal = acl_get_perm(permset, ACL_EXECUTE);
            if (permVal == -1)
                errExit("acl_get_perm - ACL_EXECUTE");
            printf("%c", (permVal == 1) ? 'x' : '-');
	    if (permVal == 0)
		x = 0;

            printf("\n");

	}
	found = 0;
    }
    printf("User/Group permission after considering ACL_MASK:");
    printf("%c", (r == 1) ? 'r' : '-');
    printf("%c", (w == 1) ? 'w' : '-');
    printf("%c", (x == 1) ? 'x' : '-');
    printf("\n");

    if (acl_free(acl) == -1)
        errExit("acl_free");    
    

    
    exit(EXIT_SUCCESS);
}


uid_t           /* Return UID corresponding to 'name', or -1 on error */
userIdFromName(const char *name)
{
    struct passwd *pwd;
    uid_t u;
    char *endptr;

    if (name == NULL || *name == '\0')  /* On NULL or empty string */
        return -1;                      /* return an error */

    u = strtol(name, &endptr, 10);      /* As a convenience to caller */
    if (*endptr == '\0')                /* allow a numeric string */
        return u;

    pwd = getpwnam(name);
    if (pwd == NULL)
        return -1;

    return pwd->pw_uid;
}

gid_t           /* Return GID corresponding to 'name', or -1 on error */
groupIdFromName(const char *name)
{
    struct group *grp;
    gid_t g;
    char *endptr;

    if (name == NULL || *name == '\0')  /* On NULL or empty string */
        return -1;                      /* return an error */

    g = strtol(name, &endptr, 10);      /* As a convenience to caller */
    if (*endptr == '\0')                /* allow a numeric string */
        return g;

    grp = getgrnam(name);
    if (grp == NULL)
        return -1;

    return grp->gr_gid;
}


