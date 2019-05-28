/*
 * Implement realpath().
 *
 *
 *
 * $ ../dirs_links/t_realpath ../test/y
 * realpath: ../test/y --> /home/x/tlpi_ex/test/x
 * $ ../dirs_links/t_realpath y
 * realpath: y --> /home/x/tlpi_ex/test/x
 * $ ../dirs_links/t_realpath ./x
 * realpath: ./x --> /home/x/tlpi_ex/test/x
 * $ ../dirs_links/t_realpath ../dirs_links/t_realpath
 * realpath: ../dirs_links/t_realpath --> /home/x/tlpi_ex/dirs_links/t_realpath
 *
 *
 */


#include <sys/stat.h>
#include <limits.h>         /* For definition of PATH_MAX */
#include <strings.h>
#include <libgen.h>
#include "tlpi_hdr.h"

#define BUF_SIZE PATH_MAX

char 
*resolve_dots(const char *pathname);

char 
*t_realpath(const char *pathname, char *resolved_path);

int
main(int argc, char *argv[])
{
    char buf[BUF_SIZE];

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname\n", argv[0]);

    if (t_realpath(argv[1], buf) == NULL)
        errExit("realpath");
    printf("realpath: %s --> %s\n", argv[1], buf);

    exit(EXIT_SUCCESS);
}


char *t_realpath(const char *pathname, char *resolved_path)
{
    ssize_t  numBytes;
    struct stat statbuf;
    char *spathname;
    char *linkpath = malloc(PATH_MAX);
    char *linkbuf =  malloc(PATH_MAX);
    char *tpathname =  malloc(PATH_MAX);
    if(resolved_path==NULL)
	resolved_path = malloc(PATH_MAX);
    if (lstat(pathname, &statbuf) == -1)
        errExit("lstat");
    spathname = resolve_dots(pathname);
    if (!S_ISLNK(statbuf.st_mode))
    {
	strcpy(resolved_path, spathname);
	free(spathname);
	return resolved_path;
    }
    numBytes = readlink(spathname, linkbuf, BUF_SIZE - 1);
    if (numBytes == -1)
        errExit("readlink");
    linkbuf[numBytes] = '\0';
    free(spathname);

    strcpy(tpathname, pathname);
    /*dirname does not take const char* */
    strcpy(linkpath, dirname(tpathname));
    strcat(linkpath, "/");
    strcat(linkpath, linkbuf);

    spathname = resolve_dots(linkpath);
    strcpy(resolved_path, spathname);

    free(linkbuf);
    free(spathname);
    free(tpathname);
    free(linkpath);

    return resolved_path;
}



char *resolve_dots(const char *pathname)
{
    struct stat statbuf;
    int pathname_len, i, j, c;
    char *spathname = (char *)malloc(PATH_MAX);
    char *temp = malloc(PATH_MAX);
    char **arr = malloc(PATH_MAX * sizeof(char *));
    getcwd(spathname, PATH_MAX);
    strcat(spathname, "/");
    strcat(spathname, pathname);
    if(spathname[strlen(spathname)-1]=='.')
    {
	spathname[strlen(spathname)] = '/';
	spathname[strlen(spathname)+1] = '\0';
    }
    pathname_len = strlen(spathname);
    if (lstat(pathname, &statbuf) == -1)
        errExit("lstat");

    c = 0;
    j = 0;
    for(i =0; i < pathname_len; i++)
    {
	if(spathname[i]=='/')
	{
	    if(c==0)
	        continue;
	    temp[c] = '\0';
	    arr[j++] = temp;
	    c = 0;
	    temp = malloc(PATH_MAX);
	}
	else if(spathname[i]=='.')
	{
	    if((i>0) && (i+1<=pathname_len) && (spathname[i-1] !='/'))
	        fatal("%s is not a valid path", pathname);
	    if(spathname[i+1]=='/') /*Check for ./ */
		i++;
	    /* Check for ../ */
	    else if((i+2<=pathname_len)&&(spathname[i+1]=='.') && (spathname[i+2]=='/'))
	    {
		i+=2;
		free(arr[j-1]); /* Jump up one directory */
		j--;
		if(j<0)
	            fatal("%s is not a valid path", pathname);
	    }
	    else
	        fatal("%s is not a valid path", pathname);
	}
	else
	{
	    temp[c++] = spathname[i];
	}
    }
    if(c!=0)
    {
	temp[c] = '\0';
	arr[j++] = temp;
    }
    free(spathname);
    spathname = malloc(PATH_MAX);
    strcpy(spathname,"");
    for(i=0; i< j; i++)
    {
	strcat(spathname, "/");
	strcat(spathname, arr[i]);
	free(arr[i]);
    }
    free(arr);

    return spathname;
}

    
