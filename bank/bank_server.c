#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"


#define MESSAGE_PREFIX "[SERVER]"

#define BUF_SIZE (256)


int main(int argc, char** argv)
{
    int pport;
    int sockfd;
    int sockconfd;
    int pollablefd;
    int listenr;
    int acceptr;
    int pollr;
    int readr;
    int sendr;

    char* rbuf;
    char* wbuf;

    appmode_t mode = { 0 };

    struct sockaddr_in saddr = { 0 };
    struct sockaddr_in caddr = { 0 };

    socklen_t caddr_len;

    struct pollfd pfd = { 0 };

    rbuf = (char*)calloc(BUF_SIZE, sizeof(char));
    wbuf = (char*)calloc(BUF_SIZE, sizeof(char));


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
    
    print_appmode(&mode);


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
            // printf("listening\n");
            
            acceptr = accept(sockfd, NULL, NULL); // block me until connection is made, returns new connection fd
            if (listen < 0)
                goto error;         // poor error handling need to improve
           
            pollablefd = acceptr;

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

            pollablefd = sockfd;

            break;

        default:
            goto error; // should be an impossibility to get here
    }

    // STEP 3
    // Server loop wait for the socket fd (either socket FD directly or
    // specific connection fd, depends on the protocol for data)
    // 

    pfd.fd      = pollablefd;
    pfd.events  = POLLIN;
    pfd.revents = 0;

    do {
        pollr = poll(&pfd, 1, -1);

        // handle poll stuff
        if (pollr > 0) {
            if (pfd.revents & ( POLLNVAL | POLLERR | POLLHUP ))  {
                goto error;

            } else {
                // must be done in a protocol specific fashion
                // `recv` is fine for TCP
                // `recvfrom` is required for UDP because I don't know where
                // unless I get it from incoming message for later `sendto`
                switch (mode.proto) {
                    case TCP:
                        readr = recv(pollablefd, rbuf, BUF_SIZE, 0);    // PROPER ERROR HANDLING
                        break;
                    case UDP:
                        caddr_len = sizeof(caddr);                      // initialized
                        readr = recvfrom(pollablefd, rbuf, BUF_SIZE, 0,
                            (struct sockaddr*)&caddr, &caddr_len);      // PROPER ERROR HANDLING
                        // printf("RECVFROM: %d\n", readr);
                        printf("PEER ADDR STUFF: %d, %d, %d\n", caddr.sin_addr.s_addr, caddr.sin_port, caddr_len);
                        break;
                }

                // read into a buffer & then parse (generic actions)
                rbuf[readr] = '\0';
                printf("%s\n", rbuf);

                // protocol specific reply (then continue listening)
                switch (mode.proto) {
                    case TCP:
                        sendr = send(pollablefd, "reply", 5, 0);  // PROPER ERROR HANDLING
                        break;
                    case UDP:
                        sendr = sendto(pollablefd, "reply", 5, 0,
                            (struct sockaddr*)&caddr, caddr_len); // PROPER ERROR HANDLING
                        break;
                }
            }

        } else {
            printf("error with poll\n");
            goto error;
        } // don't have to handle timeout because timeout is infinite

        printf("uh oh\n");

    } while (1);

    printf("server program terminating\n");
    
    close(pollablefd);

    free(rbuf);
    free(wbuf);

    return 0;

error:
    printf("error running server program exiting\n");

    close(pollablefd);

    free(rbuf);
    free(wbuf);

    return 1;
}

