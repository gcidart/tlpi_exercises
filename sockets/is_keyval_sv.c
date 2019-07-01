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
    struct kvp * next;
};



int
main(int argc, char *argv[])
{
    struct sockaddr_storage claddr;
    int lfd, cfd;
    socklen_t addrlen;
#define ADDRSTRLEN (NI_MAXHOST + NI_MAXSERV + 10)
    char addrStr[ADDRSTRLEN];
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    struct request req;
    struct response res;

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [init-seq-num]\n", argv[0]);

    struct kvp *head, *temp, *prev;
    head = malloc(sizeof (struct kvp));
    if(head==NULL)
        fatal("malloc failure");

        /* Ignore the SIGPIPE signal, so that we find out about broken connection
       errors via a failure from write(). */

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)    errExit("signal");

    lfd = inetListen(PORT_NUM, BACKLOG, &addrlen); 
    if(lfd==-1)
        errExit("inetListen");
    
    for (;;) {                  /* Handle clients iteratively */

        /* Accept a client connection, obtaining client's address */

        cfd = accept(lfd, (struct sockaddr *) &claddr, &addrlen);
        if (cfd == -1) {
            errMsg("accept");
            continue;
        }

        if (getnameinfo((struct sockaddr *) &claddr, addrlen,
                    host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
            snprintf(addrStr, ADDRSTRLEN, "(%s, %s)", host, service);
        else
            snprintf(addrStr, ADDRSTRLEN, "(?UNKNOWN?)");
        printf("Connection from %s\n", addrStr);

        /* Read client request */
    
        if(read(cfd, &req, sizeof(struct request)) <=0)
        {
            close(cfd);
            continue;
        }

        if(req.type== KV_ADD)
        {
            temp = malloc(sizeof(struct kvp));
            if(temp==NULL)
                fatal("malloc failure");
            strcpy(temp->key, req.key);
            strcpy(temp->val, req.val);
            strcpy(res.key, temp->key);
            strcpy(res.val, temp->val);
            temp->next = head->next;
            head->next = temp;
        }
        else 
        {
            temp = head->next;
            prev = head;
            while(temp!=NULL)
            {
                if(strcmp(temp->key, req.key)==0)
                    break;
                temp = temp->next;
                prev = prev->next;
            }
            if(temp!=NULL)
            {
                if(req.type==KV_MOD)
                {
                    strcpy(temp->val, req.val);
                    strcpy(res.key, temp->key);
                    strcpy(res.val, temp->val);
                }
                else if(req.type==KV_GET)
                {   
                    strcpy(res.key, temp->key);
                    strcpy(res.val, temp->val);
                }
                else if(req.type==KV_DEL)
                {   
                    prev->next = temp->next;
                    free(temp);
                    strcpy(res.key, req.key);
                    strcpy(res.val, "");
                }
                else
                {
                    strcpy(res.key,"");
                    strcpy(res.val,"");
                }
            }
            else
            {   
                strcpy(res.key, "");
                strcpy(res.val, "");
            }
        }


        if (write(cfd, &res, sizeof(struct response))<0 )
            fprintf(stderr, "Error on write\n");


        if (close(cfd) == -1)           /* Close connection */
            errMsg("close");
    }
}
