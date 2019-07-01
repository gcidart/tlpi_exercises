/*
 *Write a network server that stores name-value pairs. 
 *The server should allow names to be added, deleted, modified, and retrieved by clients.
 *Write one or more client programs to test the server. 
 *Optionally, implement some kind of security mechanism that allows only the client that created the name to delete it or to modify the value associated with it.
 * 
 * $ ./is_keyval_sv &
 * [1] 22410
 * $ ./is_keyval_cl -g localhost ch1
 * Connection from (localhost, 59224)
 * Key: ch1 not found
 * $ ./is_keyval_cl -a localhost ch1 history
 * Connection from (localhost, 59226)
 * Added Key: ch1 with value: history
 * $ ./is_keyval_cl -g localhost ch1
 * Connection from (localhost, 59228)
 * Found Key: ch1 with value: history
 * $ ./is_keyval_cl -d localhost ch1
 * Connection from (localhost, 59230)
 * Deleted Key: ch1 
 * $ ./is_keyval_cl -g localhost ch1
 * Connection from (localhost, 59232)
 * Key: ch1 not found
 * $ ./is_keyval_cl -a localhost ch1 history
 * Connection from (localhost, 59234)
 * Added Key: ch1 with value: history
 * $ ./is_keyval_cl -a localhost ch2 funda
 * Connection from (localhost, 59236)
 * Added Key: ch2 with value: funda
 * $ ./is_keyval_cl -a localhost ch3 syspro
 * Connection from (localhost, 59238)
 * Added Key: ch3 with value: syspro
 * $ ./is_keyval_cl -a localhost ch4 uniio
 * Connection from (localhost, 59240)
 * Added Key: ch4 with value: uniio
 * $ ./is_keyval_cl -g localhost ch3
 * Connection from (localhost, 59242)
 * Found Key: ch3 with value: syspro
 * $ ./is_keyval_cl -d localhost ch3
 * Connection from (localhost, 59244)
 * Deleted Key: ch3 
 * $ ./is_keyval_cl -g localhost ch1
 * Connection from (localhost, 59246)
 * Found Key: ch1 with value: history
 * $ ./is_keyval_cl -g localhost ch2
 * Connection from (localhost, 59248)
 * Found Key: ch2 with value: funda
 * $ ./is_keyval_cl -g localhost ch3
 * Connection from (localhost, 59250)
 * Key: ch3 not found
 * $ ./is_keyval_cl -g localhost ch4
 * Connection from (localhost, 59252)
 * Found Key: ch4 with value: uniio
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

#define KEY_SIZE 10
#define VAL_SIZE 100

#define KV_ADD 1
#define KV_MOD 2
#define KV_DEL 3
#define KV_GET 4

struct request
{
    int type;
    char key[KEY_SIZE];
    char val[VAL_SIZE];    
};

struct response
{
    char key[KEY_SIZE];
    char val[VAL_SIZE];
};


struct kvp
{
    char key[KEY_SIZE];
    char val[VAL_SIZE];
    struct kvp * nxt;
};

static void
usageError(const char *progName)
{
    fprintf(stderr, "Usage: %s -a|m|d|g server key val\n", progName);
    fprintf(stderr, "    -a           Add key-val pair\n");
    fprintf(stderr, "    -m           Modify entry mapped to ket to have val\n");
    fprintf(stderr, "    -d           Delete key\n");
    fprintf(stderr, "    -g           Get val for key\n");
    exit(EXIT_FAILURE);
}

 



int
main(int argc, char *argv[])
{
    int cfd, opt;
    struct request req;
    struct response res;
    


    while ((opt = getopt(argc, argv, "amdg")) != -1) {
        switch (opt) {
        case 'a':   req.type = KV_ADD;        break;
        case 'm':   req.type = KV_MOD;        break;
        case 'd':   req.type = KV_DEL;        break;
        case 'g':   req.type = KV_GET;        break;
        default:    usageError(argv[0]);
        }
    }

    if (optind +1>= argc)
        usageError(argv[0]);

    strcpy(req.key, argv[optind+1]);
    if((req.type==KV_ADD) || (req.type==KV_MOD))
    {
        if(optind+3!=argc)
            usageError(argv[0]);
        else
            strcpy(req.val,argv[optind+2]);
    }
    else
        strcpy(req.val,"");




    cfd = inetConnect(argv[optind], PORT_NUM, SOCK_STREAM);
    if(cfd==-1)
        errExit("inetConnect");
    /* Send request */

    if (write(cfd, &req, sizeof(struct request)) <=0)
        fatal("Partial/failed write ");
 
    /* Read response returned by server */
    if(read(cfd, &res, sizeof(struct response))<=0)
        fatal("Unexpected error from server");
    
    if(req.type==KV_ADD)
    {
        printf("Added Key: %s with value: %s\n", res.key, res.val);
    }
    else if(req.type==KV_MOD)
    {
        if(strcmp(req.key,res.key)==0)
            printf("Key: %s has value: %s mapped to it\n", res.key, res.val);
        else
            printf("Key: %s not found\n", req.key);
    }
    else if(req.type==KV_DEL)
    {
        if(strcmp(req.key,res.key)==0)
            printf("Deleted Key: %s \n", res.key);
        else
            printf("Key: %s not found\n", req.key);
    }
    else if(req.type==KV_GET)
    {
        if(strcmp(req.key,res.key)==0)
            printf("Found Key: %s with value: %s\n", res.key, res.val);
        else
            printf("Key: %s not found\n", req.key);
    }
    


    exit(EXIT_SUCCESS);                         /* Closes 'cfd' */
}
