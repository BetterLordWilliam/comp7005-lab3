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


enum ret_bankserver {
    BANKSERV_OK,
    BANKSERV_ERR
};


/**
*/
int bank_server_tcp(appmode_t* am)
{
    int sockfd;
    int consockfd;
    int listenr;
    int acceptr;
    int pollr;
    int sendr;
    int readr;

    struct sockaddr_in saddr    = { 0 };
    struct pollfd pfd           = { 0 };

    char* rbuf = (char*)calloc(BUF_SIZE, sizeof(char));

    // 1 socket setup
    if (getsockfd_tcp(&sockfd) > 0)
        goto error;
    setsockaddr_lb(&saddr, am->port); // sockaddr -> lb:port
    if (bindsock(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)))
        goto error;
    listenr = listen(sockfd, 1); // double check connection queue size
    if (listenr != 0)
        goto error;         // poor error handling need to improve
    printf("listening\n");
    consockfd = accept(sockfd, NULL, NULL); // block me until connection is made, returns new connection fd
    if (listen < 0)
        goto error;         // poor error handling need to improve

    // 2 server poll loop
    pfd.fd      = consockfd;
    pfd.events  = POLLIN;
    pfd.revents = 0;

    do {
        pollr = poll(&pfd, 1, -1);

        // handle poll stuff
        if (pollr > 0) {
            if (pfd.revents & ( POLLNVAL | POLLERR | POLLHUP ))  {
                goto error;
            } else {
                readr = recv(pfd.fd, rbuf, BUF_SIZE, 0);    // PROPER ERROR HANDLING

                // read into a buffer & then parse (generic actions)
                rbuf[readr] = '\0';
                printf("%s\n", rbuf);
                
                // app logic (common)
                // should & will be its own function

                // send the reply
                sendr = send(pfd.fd, "reply", 5, 0);  // PROPER ERROR HANDLING
            }
        } else {
            printf("error with poll\n");
            goto error;
        } // don't have to handle timeout because timeout is infinite

        printf("uh oh\n");

    } while (1);

    printf("server program terminating\n");

    close(sockfd);

    free(rbuf);

    return BANKSERV_OK;

error:
    close(sockfd);

    free(rbuf);

    return BANKSERV_ERR;
}

/**
*/
int bank_server_udp(appmode_t* am)
{
    int sockfd;
    int consockfd;
    int listenr;
    int acceptr;
    int pollr;
    int sendr;
    int readr;
    
    struct sockaddr_in saddr    = { 0 };
    struct sockaddr_in caddr    = { 0 };        // peers address
    struct pollfd pfd           = { 0 };

    socklen_t caddr_len = sizeof(caddr);

    char* rbuf = (char*)calloc(BUF_SIZE, sizeof(char));

    // 1 socket setup
    if (getsockfd_udp(&sockfd) > 0)
        goto error;
    // printf("%d\n", sockfd);
    setsockaddr_lb(&saddr, am->port); // sockaddr -> lb:port
    // printf("%d, %hd\n", saddr.sin_addr.s_addr, saddr.sin_port);
    if (bindsock(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)))
        goto error;

    // 2 server poll loop
    pfd.fd      = sockfd;
    pfd.events  = POLLIN;
    pfd.revents = 0;

    do {
        pollr = poll(&pfd, 1, -1);

        // handle poll stuff
        if (pollr > 0) {
            if (pfd.revents & ( POLLNVAL | POLLERR | POLLHUP ))  {
                goto error;

            } else {
                readr = recvfrom(sockfd, rbuf, BUF_SIZE, 0,
                    (struct sockaddr*)&caddr, &caddr_len);      // PROPER ERROR HANDLING
                // printf("RECVFROM: %d\n", readr);
                printf("PEER ADDR STUFF: %d, %d, %d\n", caddr.sin_addr.s_addr, caddr.sin_port, caddr_len);

                // read into a buffer & then parse (generic actions)
                rbuf[readr] = '\0';
                printf("%s\n", rbuf);

                sendr = sendto(sockfd, "reply", 5, 0,
                    (struct sockaddr*)&caddr, caddr_len); // PROPER ERROR HANDLING
            }

        } else {
            printf("error with poll\n");
            goto error;
        } // don't have to handle timeout because timeout is infinite

        printf("uh oh\n");

    } while (1);

    printf("server program terminating\n");
    
    close(sockfd);
    free(rbuf);

    return BANKSERV_OK;

error:
    close(sockfd);
    free(rbuf);

    return BANKSERV_ERR;
}


int main(int argc, char** argv)
{
    int pport;
    appmode_t mode = { 0 };

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
    // arg2 port
    if ((pport = atoi(argv[2])) != 0 && test_port(pport))  {
        mode.port = htons(pport);
    } else {
        printf("%s failed to parse port to integer\n", MESSAGE_PREFIX);
        goto error;
    }
    
    print_appmode(&mode);
    printf("starting server w/ proto %d\n", mode.proto);

    switch (mode.proto) {
        case TCP:
            if (bank_server_tcp(&mode) != 0)
                goto error;
            break;
        case UDP:
            if (bank_server_udp(&mode) != 0)
                goto error;
            break;
        default:
            printf("critical server error server attempted start w/ non TCP or UDP protocol\n");
            goto error;
    }

    printf("server finished execution\n");

    return 0;

error:
    printf("error running server program exiting\n");

    return 1;
}

