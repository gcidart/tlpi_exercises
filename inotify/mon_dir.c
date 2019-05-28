/*
 * Write a program that logs all file creations, deletions, and renames under the directory named in its command-line argument.
 * The program should monitor events in all of the subdirectories under the specified directory.
 * To obtain a list of all of these subdirectories, you will need to make use of nftw() (Section 18.9). 
 * When a new subdirectory is added under the tree or a directory is deleted, the set of monitored subdirectories should be updated accordingly.
 *
 *
 *
 * $ ./inotify/mon_dir test/ &
 * Watching test using wd 1
 * Watching test/montest using wd 2
 * Watching test/montest/montest1 using wd 3
 * Watching test/montest/montest1/montest2 using wd 4
 * Watching test/dir using wd 5
 * Watching test/file_cr_de using wd 6
 * $ mkdir test/montest/montest1/montest2/montest3
 * Read 32 bytes from inotify fd
 *    wd = 4; mask = IN_CREATE 
 *        name = montest3
 * Watching test/montest/montest1/montest2/montest3 using wd 7
 * $ rmdir test/montest/montest1/montest2/montest3
 * Read 16 bytes from inotify fd
 *    wd = 7; mask = IN_DELETE_SELF 
 * Stopped watching test/montest/montest1/montest2/montest3 using wd 7
 * Read 16 bytes from inotify fd
 *    wd = 7; mask = IN_IGNORED 
 * Read 32 bytes from inotify fd
 *    wd = 4; mask = IN_DELETE 
 *        name = montest3
 * 
 */

#include <sys/inotify.h>
#include <limits.h>
#include <ftw.h>
#include "tlpi_hdr.h"

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#define MON_DIR_SIZE 100

static int inotifyFd;
static int wd_arr[MON_DIR_SIZE];
static char *wd_path[MON_DIR_SIZE];
static int wd_count=1;

static void             /* Display information from inotify_event structure */
displayInotifyEvent(struct inotify_event *i)
{
    printf("    wd =%2d; ", i->wd);
    if (i->cookie > 0)
        printf("cookie =%4d; ", i->cookie);

    printf("mask = ");
    if (i->mask & IN_CREATE)        printf("IN_CREATE ");
    if (i->mask & IN_DELETE_SELF)   printf("IN_DELETE_SELF ");
    if (i->mask & IN_DELETE)        printf("IN_DELETE ");
    if (i->mask & IN_MOVED_FROM)    printf("IN_MOVED_FROM ");
    if (i->mask & IN_MOVED_TO)      printf("IN_MOVED_TO ");
    if (i->mask & IN_IGNORED)       printf("IN_IGNORED ");
    printf("\n");

    if (i->len > 0)
        printf("        name = %s\n", i->name);
    if (i->mask & IN_CREATE)  
    {
	int wdt = i->wd;
	char *patht = wd_path[wdt];
	char *temp = malloc(PATH_MAX);
	int flags = IN_CREATE|IN_DELETE|IN_MOVE|IN_DELETE_SELF|IN_MOVE_SELF;
	strcat(patht,"/");
	strcat(patht, i->name);
	wd_arr[wd_count] = inotify_add_watch(inotifyFd, patht, flags);
        printf("Watching %s using wd %d\n", patht, wd_arr[wd_count]);
        if (wd_arr[wd_count] == -1)
            errExit("inotify_add_watch");
	strcpy(temp, patht);
	wd_path[wd_count++] = temp;

    }
    if (i->mask & IN_DELETE_SELF)
    {
	inotify_rm_watch(inotifyFd, i->wd);
	printf("Stopped watching %s using wd %d\n", wd_path[i->wd], i->wd);
    }



}


static int                      /* Function called by nftw() */
dirTree(const char *pathname, const struct stat *sbuf, int type,
        struct FTW *ftwb)
{
    int flags = IN_CREATE|IN_DELETE|IN_MOVE|IN_DELETE_SELF|IN_MOVE_SELF;
    char *temp = malloc(PATH_MAX);
    if((type==FTW_D)||(type==FTW_DNR))
    {
	wd_arr[wd_count] = inotify_add_watch(inotifyFd, pathname, flags);
        printf("Watching %s using wd %d\n", pathname, wd_arr[wd_count]);
        if (wd_arr[wd_count] == -1)
            errExit("inotify_add_watch");
	strcpy(temp, pathname);
	wd_path[wd_count++] = temp;


    }

    return 0;                                   /* Tell nftw() to continue */
}



int
main(int argc, char *argv[])
{
    char buf[BUF_LEN] __attribute__ ((aligned(8)));
    ssize_t numRead;
    char *p;
    struct inotify_event *event;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname\n", argv[0]);

    inotifyFd = inotify_init();                 /* Create inotify instance */
    if (inotifyFd == -1)
        errExit("inotify_init");

    /* add a watch for events in all subdirectories */
    if (nftw(argv[1], dirTree, 10, FTW_PHYS) == -1) {
        perror("nftw");
        exit(EXIT_FAILURE);
    }


    for (;;) {                                  /* Read events forever */
        numRead = read(inotifyFd, buf, BUF_LEN);
        if (numRead == 0)
            fatal("read() from inotify fd returned 0!");

        if (numRead == -1)
            errExit("read");

        printf("Read %ld bytes from inotify fd\n", (long) numRead);

        /* Process all of the events in buffer returned by read() */

        for (p = buf; p < buf + numRead; ) {
            event = (struct inotify_event *) p;
            displayInotifyEvent(event);

            p += sizeof(struct inotify_event) + event->len;
        }
    }

    exit(EXIT_SUCCESS);
}
