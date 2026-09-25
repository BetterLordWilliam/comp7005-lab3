#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "common.h"


#define MESSAGE_PREFIX "[CLIENT]"

#define BUF_SIZE (256)


int main(int argc, char** argv)
{
    int pport;
    int sockfd;
    int connectr;
    int pollr;
    int readr;
    int sendr;
    int recvr;

    char* wbuf;
    char* rbuf;

    // initialize application mode all fields to 0
    appmode_t mode = { 0 };

    struct sockaddr_in saddr = { 0 };
    struct pollfd pfd = { 0 };

    // buffer allocations
    rbuf = (char*)calloc(BUF_SIZE, sizeof(char));
    wbuf = (char*)calloc(BUF_SIZE, sizeof(char));

    // determine proto & port from program arguments
    // we need 3 arguments to this program
    // be strict, reject more or less

    if (argc != 3) {
        printf("%s incorrect number of arguments\n", MESSAGE_PREFIX);
        goto error;
    }
    // arg1 protocol type
    if (strcmp(_BANK__TCP_PROTO, argv[1]) == 0 ) {
        mode.proto = TCP;
    } else if (strcmp(_BANK__UDP_PROTO, argv[1]) == 0) {
        mode.proto = UDP;
    } else {
        printf("%s unknown protocol\n", MESSAGE_PREFIX);
        goto error;
    }
    // arg2 port process to short?
    if ((pport = atoi(argv[2])) != 0 && test_port(pport))  {
        mode.port = htons(pport);
    } else {
        printf("%s failed to parse port to integer\n", MESSAGE_PREFIX);
        goto error;
    }
    
    // sanity check
    print_appmode(&mode);


    // STEP 1
    // protocol specifici socket setup
    switch (mode.proto) {
        case TCP:
            // create TCP socket
            if (getsockfd_tcp(&sockfd) > 0)
                goto error;
            setsockaddr_lb(&saddr, mode.port); // sockaddr -> lb:port
            break;

        case UDP:
            // create UDP socket
            if (getsockfd_udp(&sockfd) > 0)
                goto error;
            setsockaddr_lb(&saddr, mode.port); // sockaddr -> lb:port
            break;

        default:
            goto error; // should be an impossibility to get here
    }
    
    // STEP 2 
    // connect, this is actually the same regardless of TCP/UDP (for now)
    // even though UDP is connectionless
    // this works by caching the destination socket address for future calls
    // so for application logic I can use `recv` & `send`
    connectr = connect(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
    if (connectr < 0)
        goto error;

    // STEP 3
    // client loop begins, except we at the client level are waiting for new
    // messages from stdin
    pfd.fd      = STDIN_FILENO; // stdin because we are waiting for user input (which is the message)
    pfd.events  = POLLIN;
    pfd.revents = 0;
    
    printf("connection to server established entering poll loop.\n");

    do {
        pollr = poll(&pfd, 1, -1); // poll on stdin (messages)
        
        if (pollr > 0) {
            // printf("user entered message\n");

            // figure out what the revent is from `poll` & act accordingly
            // invalid value means error exit
            if (pfd.revents & ( POLLNVAL | POLLERR | POLLHUP )) {
                goto error; 

            } else if (pfd.revents & ( POLLIN )) {
                readr = read(pfd.fd, rbuf, BUF_SIZE);
                if (readr < 0)
                    continue; // failed to read for this command
                if (readr == 0) {
                    break;
                }
                rbuf[readr] = '\0';

                // protocol dependent stuff will happen in here again
                // printf("you typed this command: %s\n", rbuf);
                sendr = send(sockfd, rbuf, BUF_SIZE, 0);
                // printf("%d\n", sendr);
                recvr = recv(sockfd, wbuf, BUF_SIZE, 0);
                printf("%s\n", wbuf); // reply from the server
            }

        } else {
            printf("error with poll\n");
            goto error;
        } // don't have to handle timeout because timeout is infinite

        printf("uh oh\n");

    } while (1);

    printf("client program terminating\n");

    close(sockfd);
        
    free(rbuf);
    free(wbuf);

    return 0;

error:
    printf("error running client program exiting\n");

    close(sockfd);

    free(rbuf);
    free(wbuf);
    return 1;
}
