/*
 * Write a program that uses nftw() to traverse a directory tree and finishes by printing out counts and percentages of the various types
 * (regular, directory, symbolic link, and so on) of files in the tree.
 */

#include <ftw.h>
#include "tlpi_hdr.h"


static int rcnt = 0;
static int dcnt = 0;
static int scnt = 0;
static int cnt = 0;

static int                      /* Function called by nftw() */
dirTree(const char *pathname, const struct stat *sbuf, int type,
        struct FTW *ftwb)
{
    if((type==FTW_D)||(type==FTW_DNR))
	dcnt++;
    if(type==FTW_F)
	rcnt++;
    if(type==FTW_SL)
	scnt++;
    cnt++;

    return 0;                                   /* Tell nftw() to continue */
}

int
main(int argc, char *argv[])
{
    int flags;

    flags = FTW_PHYS;
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname\n", argv[0]);

    if (nftw(argv[1], dirTree, 10, flags) == -1) {
        perror("nftw");
        exit(EXIT_FAILURE);
    }
    printf("%d regular files out of total %d entries\n", rcnt, cnt);
    printf("%d directories out of total %d entries\n", dcnt, cnt);
    printf("%d symbolic links out of total %d entries\n", scnt, cnt);
    exit(EXIT_SUCCESS);
}
