/* 
 * Write a program that can be used to create or modify a user EA for a file (i.e., a simple version of setfattr(1)).
 * The filename and the EA name and value should be supplied as command-line arguments to the program.  
 *
 *
 * $ getfattr -d test1/prog
 * # file: test1/prog
 * user.x="y"

 * $ ./xattr/t_setfattr test1/prog user.x yoyo 
 * $ getfattr -d test1/prog
 * # file: test1/prog
 * user.x="yoyo" * 
 */

#include <stdlib.h>
#include <sys/xattr.h>
#include "tlpi_hdr.h"




int
main(int argc, char *argv[])
{

    char *value = (char *) malloc(1024);
    strcpy(value, argv[3]);
    if (argc !=4 || strcmp(argv[1], "--help") == 0)
        usageErr("%s filename ea_name ea_value\n", argv[0]);
    
    if(setxattr(argv[1], argv[2], value, strlen(value),0)==-1)
	errExit("setxattr");

    

    
    exit(EXIT_SUCCESS);
}

