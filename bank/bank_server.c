#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"


#define MESSAGE_PREFIX "[SERVER]"


int main(int argc, char** argv)
{
    int pport, sockfd;
    appmode_t mode = { 0 };
    struct sockaddr_in saddr = { 0 };


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
        mode.port = (short)pport;
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
            printf("%d, %hd\n", saddr.sin_addr.s_addr, saddr.sin_port);

            break;
        case UDP:
            // create socket
            if (getsockfd_udp(&sockfd) > 0)
                goto error;
            // printf("%d\n", sockfd);
            // bind socket
            setsockaddr_lb(&saddr, mode.port); // sockaddr -> lb:port
            printf("%d, %hd\n", saddr.sin_addr.s_addr, saddr.sin_port);

            break;
        default:
            goto error;
    }


    return 0;

error:
    printf("error running server program exiting\n");
    return 1;
}

