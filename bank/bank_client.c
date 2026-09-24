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

    char* wbuf;
    char* rbuf;

    // initialize application mode all fields to 0
    appmode_t mode = { 0 };

    struct sockaddr_in saddr = { 0 };
    struct pollfd pfd = { 0 };


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

    // buffer allocations
    rbuf = (char*)calloc(BUF_SIZE, sizeof(char));
    wbuf = (char*)calloc(BUF_SIZE, sizeof(char));

    switch (mode.proto) {
        case TCP:
            // create TCP socket
            if (getsockfd_tcp(&sockfd) > 0)
                goto error;
            break;

        case UDP:
            // create UDP socket
            if (getsockfd_udp(&sockfd) > 0)
                goto error;
            break;

        default:
            goto error; // should be an impossibility to get here
    }


    setsockaddr_lb(&saddr, mode.port); // sockaddr -> lb:port
    connectr = connect(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
    if (connectr < 0)
        goto error;

    pfd.fd      = STDIN_FILENO; // stdin because we are waiting for user input (which is the message)
    pfd.events  = POLLIN;
    pfd.revents = 0;
    
    printf("connection to server established entering poll loop.\n");

    while (1) {
        pollr = poll(&pfd, 1, -1); // poll on stdin (messages)
        
        if (pollr > 0) {
            printf("user entered message\n");

            // figure out what the revent is from `poll` & act accordingly

            readr = read(pfd.fd, rbuf, BUF_SIZE);

            // protocol dependent stuff will happen in here again

        } else if (pollr < 0) {
            printf("error with poll\n");
            goto error;

        } else {
            // should never get here because there is infinite timeout
        }

        break;
    }

    free(rbuf);
    free(wbuf);

    return 0;

error:
    printf("error running client program exiting\n");
    free(rbuf);
    free(wbuf);
    return 1;
}
