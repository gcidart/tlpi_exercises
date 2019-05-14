/*
 Implement dup() and dup2() using fcntl() and, where necessary, close().
 (You may ignore the fact that dup2() and fcntl() return different errno values for some error cases.) 
 For dup2(), remember to handle the special case where oldfd equals newfd.
 In this case, you should check whether oldfd is valid, which can be done by, 
 for example, checking if fcntl(oldfd, F_GETFL) succeeds. 
 If oldfd is not valid, then the function should return -1 with errrno set to EBADF.
*/

#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int
t_dup(int fd);

int
t_dup2(int oldfd, int newfd);

int
main(int argc, char *argv[])
{
    int fd, fd1, fd2;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s filename  \n", argv[0]);

    fd = open(argv[1], O_RDONLY );
    if (fd == -1)
            errExit("open");

    fd1 = t_dup(fd);
    fd2 = t_dup2(fd,10);
    ssize_t offset = lseek(fd, 0 , SEEK_END);
    ssize_t offset1 = lseek(fd1, 0 , SEEK_CUR);
    ssize_t offset2 = lseek(fd1, 0 , SEEK_CUR);

    printf("Original File Handle:%d; File Offset:%ld\n", fd, (long) offset);
    printf("dup() File Handle:%d; File Offset:%ld\n", fd1, (long) offset1);
    printf("dup2() File Handle:%d; File Offset:%ld\n", fd2, (long) offset2);


    printf("Duplicating non-existent Filehandle gives a return value of %d with errno %x\n",t_dup2(5, 4), errno);
    errno = 0;

    exit(EXIT_SUCCESS);
}

int
t_dup(int fd)
{
	if(fcntl(fd, F_GETFL) == -1)
	{
		errno = EBADF;
		return -1;
	}
	return fcntl(fd, F_DUPFD, 0);
}

int
t_dup2(int oldfd, int newfd)
{
	if(fcntl(oldfd, F_GETFL) == -1)
	{
		errno = EBADF;
		return -1;
	}
	if(oldfd==newfd)
		return newfd;
	if(fcntl(newfd, F_GETFL) != -1)
	{
		close(newfd);
	}
	return fcntl(oldfd, F_DUPFD, newfd);
}



