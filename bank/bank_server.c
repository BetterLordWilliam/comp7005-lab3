#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"


#define MESSAGE_PREFIX "[SERVER]"


int main(int argc, char** argv)
{
    int pport;
    int sockfd;
    int sockconfd;
    int listenr;
    int acceptr;
    int pollr;

    appmode_t mode = { 0 };
    struct sockaddr_in saddr = { 0 };
    struct pollfd pfd = { 0 };


    // STEP 1
    // parse arguments
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
    
    // print_appmode(&mode);


    // STEP 2
    // protocol dependent socket setup

    switch (mode.proto) {

        case TCP:
            // create socket
            if (getsockfd_tcp(&sockfd) > 0)
                goto error;
            setsockaddr_lb(&saddr, mode.port); // sockaddr -> lb:port
            // printf("%d, %hd\n", saddr.sin_addr.s_addr, saddr.sin_port);
            if (bindsock(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)))
                goto error;
            // printf("bind successful\n");

            // PROTO SPECIFIC SERVER LOOP
            listenr = listen(sockfd, 1); // mark socket as passive, 1 connection in queue (double check requirements)
            if (listenr != 0)
                goto error;         // poor error handling need to improve
            printf("listening\n");
            
            acceptr = accept(sockfd, NULL, NULL); // block me until connection is made, returns new connection fd
            if (listen < 0)
                goto error;         // poor error handling need to improve
            
            // after a connection is made, unblock & enter a `poll` loop
            // will also need to create some buffer to write messages to
            // probably just one that is 256 bytes is enough as we have limited
            // message lexicon
            // for now, echo incoming messages

            pfd.fd      = acceptr;
            pfd.events  = POLLIN;
            pfd.revents = 0;
            
            printf("connection to client established entering poll loop.\n");
 
            while (1) {
                pollr = poll(&pfd, 1, -1);
                break;
            }

            break;

        case UDP:
            // create socket
            if (getsockfd_udp(&sockfd) > 0)
                goto error;
            // printf("%d\n", sockfd);
            // bind socket
            setsockaddr_lb(&saddr, mode.port); // sockaddr -> lb:port
            // printf("%d, %hd\n", saddr.sin_addr.s_addr, saddr.sin_port);
            if (bindsock(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)))
                goto error;
            // printf("bind successful\n");

            // PROTO SPECIFIC SERVER LOOP
            
            // no need to listen & accept connections for UDP
            // we can immediately jump to the `poll` loop &, for now, echo
            // incoming messages

            while (1) {
                break;
            }

            break;

        default:
            goto error; // should be an impossibility to get here
    }


    return 0;

error:
    printf("error running server program exiting\n");
    return 1;
}

