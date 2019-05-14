/*
 Implemented readv() and writev() using read() and write(), and suitable functions from the malloc package

*/
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

ssize_t
t_readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t
t_writev(int fd, const struct iovec *iov, int iovcnt);
int
main(int argc, char *argv[])
{
    int fd;
    struct iovec iov[3], wiov[3];
    struct stat myStruct, wmyStruct;       /* First buffer */
    int x, wx;                      /* Second buffer */
    #define STR_SIZE 10
    char str[STR_SIZE], wstr[STR_SIZE];         /* Third buffer */
    ssize_t numRead, numWrite, totRequired, wtotRequired;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file\n", argv[0]);

    fd = open(argv[1], O_RDWR|O_CREAT, S_IRWXU);
    if (fd == -1)
        errExit("open");

    totRequired = 0;
    wtotRequired = 0;

    stat(argv[1], &wmyStruct);
    wx = 888000;
    memcpy(wstr, "ABCDEABCDE", 10);

    iov[0].iov_base = &myStruct;
    iov[0].iov_len = sizeof(struct stat);
    totRequired += iov[0].iov_len;
    wiov[0].iov_base = &wmyStruct;
    wiov[0].iov_len = sizeof(struct stat);
    wtotRequired += wiov[0].iov_len;

    iov[1].iov_base = str;
    iov[1].iov_len = STR_SIZE+1;
    totRequired += iov[1].iov_len;
    wiov[1].iov_base = wstr;
    wiov[1].iov_len = STR_SIZE+1;
    wtotRequired += wiov[1].iov_len;

    iov[2].iov_base = &x;
    iov[2].iov_len = sizeof(x);
    totRequired += iov[2].iov_len;
    wiov[2].iov_base = &wx;
    wiov[2].iov_len = sizeof(wx);
    wtotRequired += wiov[2].iov_len;

    numWrite = t_writev(fd, wiov, 3);
    if (numWrite == -1)
        errExit("writev");

    if (numWrite < wtotRequired)
        printf("Write fewer bytes than requested\n");

    printf("total bytes requested: %ld; bytes write: %ld\n",
            (long) wtotRequired, (long) numWrite);
    lseek(fd,0, SEEK_SET);

    numRead = t_readv(fd, iov, 3);
    if (numRead == -1)
        errExit("readv");

    if (numRead < totRequired)
        printf("Read fewer bytes than requested\n");

    printf("total bytes requested: %ld; bytes read: %ld\n",
            (long) totRequired, (long) numRead);
    printf("Read back from File=> FileSize:%ld, integer:%d, string:%s\n",
		    myStruct.st_size, x, str);
    close(fd);
    exit(EXIT_SUCCESS);}

ssize_t
t_readv(int fd, const struct iovec *iov, int iovcnt)
{
	size_t tot_read_size=0;
	int ret_val;
	char *buffer;
	const struct iovec *orig = iov;
	for(int i =0; i < iovcnt; i++)
	{
		tot_read_size += iov->iov_len;
		iov++;
	}
	buffer = (char*) malloc(tot_read_size);
	iov = orig;
	
	ret_val =  read(fd, buffer, tot_read_size);
	if(ret_val!=-1)
	{
		size_t wsize = 0;
		for(int i=0; i < iovcnt;i++)
		{
			memcpy(iov->iov_base, buffer+wsize, iov->iov_len);
			wsize += iov->iov_len;
			iov++;
		}
	}
	free(buffer);
	return ret_val;

}
ssize_t
t_writev(int fd, const struct iovec *iov, int iovcnt)
{
	size_t tot_write_size=0;
	size_t wsize = 0;
	char *buffer;
	const struct iovec *orig = iov;
	for(int i =0; i < iovcnt; i++)
	{
		tot_write_size += iov->iov_len;
		iov++;
	}
	buffer = (char*) malloc(tot_write_size);
	iov = orig;
	for(int i=0; i < iovcnt;i++)
	{
		memcpy(buffer+wsize, (char*)iov->iov_base, iov->iov_len);
		wsize += iov->iov_len;
		iov++;
	}
	ssize_t ret = write(fd, buffer, tot_write_size);
	free(buffer);


	return ret;
}
