/*Write a program that opens and existing file for writing with the O_APPEND flag,
 *  and then seeks to the beginning of the file before writing some data
 *
 *  Data is still appended to the end of the file.
 *
 *  From http://man7.org/linux/man-pages/man2/open.2.html:
 *  The file is opened in append mode.  Before each write(2), the
 *  file offset is positioned at the end of the file, as if with
 *  lseek(2).  The modification of the file offset and the write
 *  operation are performed as a single atomic step.
*/
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int fd;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname \n", argv[0]);

    fd = open(argv[1], O_RDWR | O_APPEND);
    if (fd == -1)
        errExit("open");

    if (lseek(fd, 0, SEEK_SET) == -1)
        errExit("lseek");

    if (write(fd, "test", 4) == -1)
        errExit("write");
    exit(EXIT_SUCCESS);
}
