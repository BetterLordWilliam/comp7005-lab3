#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "common.h"


#define MESSAGE_PREFIX "[CLIENT]"


int main(int argc, char** argv)
{
    int pport;
    int sockfd;
    int connectr;
    int pollr;

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


    switch (mode.proto) {
        case TCP:
            // create socket
            if (getsockfd_tcp(&sockfd) > 0)
                goto error;
            setsockaddr_lb(&saddr, mode.port); // sockaddr -> lb:port

            // skip binding w/ the client
            // printf("%d, %hd\n", saddr.sin_addr.s_addr, saddr.sin_port);
            // if (bindsock(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)))
            //    goto error;
            // printf("bind successful\n");

            connectr = connect(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
            if (connectr < 0)
                goto error;

            pfd.fd      = sockfd;
            pfd.events  = POLLIN;
            pfd.revents = 0;
            
            printf("connection to server established entering poll loop.\n");
 
            while (1) {
                pollr = poll(&pfd, 1, -1);
                break;
            }

            break;

        case UDP:
            // create socket
            if (getsockfd_udp(&sockfd) > 0)
                goto error;
            setsockaddr_lb(&saddr, mode.port); // sockaddr -> lb:port

            // skip binding w/ the client
            // printf("%d, %hd\n", saddr.sin_addr.s_addr, saddr.sin_port);
            // if (bindsock(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)))
            //    goto error;
            // printf("bind successful\n");

            connectr = connect(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
            if (connectr < 0)
                goto error;

            pfd.fd      = sockfd;
            pfd.events  = POLLIN;
            pfd.revents = 0;
            
            printf("connection to server established entering poll loop.\n");
 
            while (1) { // do I need to do this?
                pollr = poll(&pfd, 1, -1); // yes poll (block process while there's nothing to do)
                // recvfrom
                break;
            }

            break;

        default:
            goto error;
    }


    return 0;

error:
    printf("error running client program exiting\n");
    return 1;
}
