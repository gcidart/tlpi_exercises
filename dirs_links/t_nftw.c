/*
 *
 * Implement nftw(). (This will require the use of the opendir(), readdir(), closedir(), and stat() system calls, among others.)
 *
 *
 *
 * t_nftw() emulates limited functionality of nftw()
 * nopenfd and flags arguments for the function are not used.
 * Test for t_nftw() uses the textbook code from dirs_links/nftw_dir_tree.c
 *
 * $ ./dirs_links/t_nftw test/
 * d D     657857  test
 * - F     668603      x
 * - F     657872      sparse_file_for_t_cp
 * - F     668576      test_tail.txt
 * - F     658083      file
 * - F     668604      abc
 * d D    1454480      montest
 * d D    1454481          montest1
 * d D    1454478              montest2
 * - F     668586      prog
 * d D    1454382      dir
 * d D    1454377      file_cr_de
 * l SL    668605      y
 * 
 *
 */



#include <ftw.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <libgen.h>
#include "tlpi_hdr.h"


static int 
t_nftw(const char *dirpath,
         int (*func) (const char *pathname, const struct stat *statbuf,
                     int typeflag, struct FTW *ftwbuf),
         int nopenfd, int flags)
{
	static int level = 0;
	DIR *dirp;
	int len,flag=0;
	struct stat statbuf ;
	struct FTW ftwbuf;
    struct dirent *dp;
	char* newdirpath = malloc(PATH_MAX);
	strcpy(newdirpath, dirpath);
	if(newdirpath[strlen(newdirpath)-1]=='/')
		newdirpath[strlen(newdirpath)-1] = '\0';
    len = strlen(dirname(newdirpath));

	ftwbuf.base = (len!=1)?len+1:0;
	ftwbuf.level = level;
	if(lstat(dirpath, &statbuf)==-1)
	{
		flag|=FTW_NS;
	}
	switch (statbuf.st_mode & S_IFMT) {  /* Print file type */
        case S_IFREG:  flag|=FTW_F; break;
        case S_IFDIR:  flag|=FTW_D; break;
        case S_IFCHR:  flag|=FTW_F; break;
        case S_IFBLK:  flag|=FTW_F; break;
        case S_IFLNK:  flag|=FTW_SL; break;
        case S_IFIFO:  flag|=FTW_F; break;
        case S_IFSOCK: flag|=FTW_F; break;
        }
	func(newdirpath, &statbuf, flag,&ftwbuf);
	free(newdirpath);
	dirp = opendir(dirpath);
    if (dirp  == NULL) {
        return 0;
	}
	for (;;) {
        errno = 0;              /* To distinguish error from end-of-directory */
        dp = readdir(dirp);
        if (dp == NULL)
            break;

        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;           /* Skip . and .. */

        newdirpath = malloc(PATH_MAX);
		strcpy(newdirpath, dirpath);
		if(newdirpath[strlen(newdirpath)-1]=='/')
			newdirpath[strlen(newdirpath)-1] = '\0';
		strcat(newdirpath,"/");
		strcat(newdirpath,dp->d_name);
		level++;
		t_nftw(newdirpath, func, nopenfd, flags);
		level--;
		free(newdirpath);

    }
    if (errno != 0)
	{
        errExit("readdir");
	}

    if (closedir(dirp) == -1)
	{
        errMsg("closedir");
	}

	return 0;
}

	



static int                      /* Function called by nftw() */
dirTree(const char *pathname, const struct stat *sbuf, int type,
        struct FTW *ftwb)
{
    if (type == FTW_NS) {                  /* Could not stat() file */
        printf("?");
    } else {
        switch (sbuf->st_mode & S_IFMT) {  /* Print file type */
        case S_IFREG:  printf("-"); break;
        case S_IFDIR:  printf("d"); break;
        case S_IFCHR:  printf("c"); break;
        case S_IFBLK:  printf("b"); break;
        case S_IFLNK:  printf("l"); break;
        case S_IFIFO:  printf("p"); break;
        case S_IFSOCK: printf("s"); break;
        default:       printf("?"); break; /* Should never happen (on Linux) */
        }
    }

    printf(" %s  ", (type == FTW_D)  ? "D  " : (type == FTW_DNR) ? "DNR" :
            (type == FTW_DP) ? "DP " : (type == FTW_F)   ? "F  " :
            (type == FTW_SL) ? "SL " : (type == FTW_SLN) ? "SLN" :
            (type == FTW_NS) ? "NS " : "  ");

    if (type != FTW_NS)
        printf("%7ld ", (long) sbuf->st_ino);
    else
        printf("        ");

    printf(" %*s", 4 * ftwb->level, "");        /* Indent suitably */
    printf("%s\n",  &pathname[ftwb->base]);     /* Print basename */
    return 0;                                   /* Tell nftw() to continue */
}

int
main(int argc, char *argv[])
{
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname...\n", argv[0]);
    if (t_nftw(argv[1], dirTree, 10, FTW_PHYS) == -1) {
        perror("nftw");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
