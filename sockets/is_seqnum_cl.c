/*
 *When reading large quantities of data, the readLine() function shown in Listing 59-1 is inefficient, since a system call is required to read each character.
 *A more efficient interface would read a block of characters into a buffer and extract a line at a time from this buffer.
 *Such an interface might consist of two functions.
 *The first of these functions, which might be called readLineBufInit(fd, &rlbuf), initializes the bookkeeping data structure pointed to by rlbuf.
 *This structure includes space for a data buffer, the size of that buffer, and a pointer to the next “unread” character in that buffer.
 *It also includes a copy of the file descriptor given in the argument fd.
 *The second function, readLineBuf(&rlbuf), returns the next line from the buffer associated with rlbuf.
 *If required, this function reads a further block of data from the file descriptor saved in rlbuf.
 *Implement these two functions. Modify the programs in Listing 59-6 (is_seqnum_sv.c) and Listing 59-7 (is_seqnum_cl.c) to use these functions.
 * 
 * 
 *Modify the programs in Listing 59-6 (is_seqnum_sv.c) and Listing 59-7 (is_seqnum_cl.c) to use the inetListen() and inetConnect() functions provided in Listing 59-9 (inet_sockets.c).
 * 
 * 
 * 
 * $ ./is_seqnum_sv &
 * [1] 4798
 * $ ./is_seqnum_cl localhost 1
 * Connection from (localhost, 58994)
 * Sequence number: 0
 * $ ./is_seqnum_cl localhost 10
 * Connection from (localhost, 58996)
 * Sequence number: 1
 * $ ./is_seqnum_cl localhost 1
 * Connection from (localhost, 58998)
 * Sequence number: 11
 * 
*/

#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "tlpi_hdr.h"
#include "inet_sockets.h"       /* Declarations of inet*() socket functions */

#define PORT_NUM "50000"        /* Port number for server */

#define INT_LEN 30              /* Size of string able to hold largest
                                   integer (including terminating '\n') */
#define BACKLOG 50
#define BUFF_SIZE 10
struct rlbuf
{
    char buf[BUFF_SIZE];
    char *nptr;
    int fd;
};

static void
readLineBufInit(int fd, struct rlbuf * rlb)
{
    rlb->fd = fd;
    rlb->nptr = rlb->buf+BUFF_SIZE;
}
static char *
readLineBuf(struct rlbuf *rlb)
{
    char *buffer = malloc(BUFF_SIZE);
    size_t sz = BUFF_SIZE;
    size_t i = 0;
    for(;;)
    {
        if(rlb->nptr >= rlb->buf + BUFF_SIZE)
        {
            if(read(rlb->fd, rlb->buf, BUFF_SIZE)<=0)
            {
                rlb->nptr = rlb->buf + BUFF_SIZE;
                return NULL;
            }
            rlb->nptr = rlb->buf;
        }
        buffer[i++]=*(rlb->nptr);
        rlb->nptr++;
        if(i==(sz-1))
        {
            if(realloc(buffer, 2*sz)==NULL)
                return NULL;
            sz = sz*2;
        }
        if(buffer[i-1]=='\n')
        {
            buffer[i] = '\0';
            return buffer;
        }
    }
    return NULL;
}
    



int
main(int argc, char *argv[])
{
    char *reqLenStr;                    /* Requested length of sequence */
    char *seqNumStr;                   /* Start of granted sequence */
    int cfd;
    struct rlbuf rb;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s server-host [sequence-len]\n", argv[0]);

    cfd = inetConnect(argv[1], PORT_NUM, SOCK_STREAM);
    if(cfd==-1)
        errExit("inetConnect");
    /* Send requested sequence length, with terminating newline */

    reqLenStr = (argc > 2) ? argv[2] : "1";
    if (write(cfd, reqLenStr, strlen(reqLenStr)) !=  strlen(reqLenStr))
        fatal("Partial/failed write (reqLenStr)");
    if (write(cfd, "\n", 1) != 1)
        fatal("Partial/failed write (newline)");
    
    readLineBufInit(cfd, &rb);
    /* Read and display sequence number returned by server */

    seqNumStr = readLineBuf(&rb);
    if(seqNumStr == NULL)
        fatal("Unexpected error from server");

    printf("Sequence number: %s", seqNumStr);   /* Includes '\n' */

    free(seqNumStr);

    exit(EXIT_SUCCESS);                         /* Closes 'cfd' */
}
