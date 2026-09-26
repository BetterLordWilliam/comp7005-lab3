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


/**
Takes message from the client & processes the request writes a response message.
    bank_t* bank pointer to bank struct (bank state)
    char* request
    char* response
*/
int bank_processing(bank_t* bank, const char* request, char* response) {
    printf("%s\n", request);
    int t;

    if (strncmp(_BANK__BALANCE_MSG_PREFIX,
            request, strlen(_BANK__BALANCE_MSG_PREFIX)) == 0) {

        sprintf(response, _BANK__REPLY_MSG_PREFIX " " _BANK__BALANCE_MSG_PREFIX " %d",
            bank->balance);

    } else if (strncmp(_BANK__DEPOSIT_MSG_PREFIX,
            request, strlen(_BANK__DEPOSIT_MSG_PREFIX)) == 0) {

        sscanf(request, _BANK__DEPOSIT_MSG_PREFIX "  %d", &t);
        bank->balance += t;
        sprintf(response, _BANK__REPLY_MSG_PREFIX " " _BANK__BALANCE_MSG_PREFIX " %d",
            bank->balance);

    } else if (strncmp(_BANK__WITHDRAW_MSG_PREFIX,
            request, strlen(_BANK__WITHDRAW_MSG_PREFIX)) == 0) {

        sscanf(request, _BANK__WITHDRAW_MSG_PREFIX " %d", &t);
        if (t < bank->balance)
            bank->balance -= t;
        sprintf(response, _BANK__REPLY_MSG_PREFIX " " _BANK__BALANCE_MSG_PREFIX " %d",
            bank->balance);

    } else if (strncmp(_BANK__QUIT_MSG,
            request, strlen(_BANK__QUIT_MSG)) == 0) {

        sprintf(response, _BANK__SERVER_SHUTDOWN_MSG_PREFIX);

    } else {
        printf("unknown message type received\n");
    }

    return 0; 
}


enum ret_bankserver {
    BANKSERV_OK,
    BANKSERV_ERR
};


/**
*/
int bank_server_tcp(appmode_t* am, bank_t* bank)
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

    char* rbuf = (char*)calloc(_BANK__BUF_SIZE, sizeof(char));
    char* wbuf = (char*)calloc(_BANK__BUF_SIZE, sizeof(char));

    // 1 socket setup
    if (getsockfd_tcp(&sockfd) > 0)
        goto error;
    setsockaddr_lb(&saddr, am->port); // sockaddr -> lb:port
    if (bindsock(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)))
        goto error;
    listenr = listen(sockfd, 1); // double check connection queue size
    if (listenr != 0)
        goto error;
    printf("listening\n");
    consockfd = accept(sockfd, NULL, NULL); // block me until connection is made, returns new connection fd
    if (consockfd < 0)
        goto error;

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
                readr = recv(pfd.fd, rbuf, _BANK__BUF_SIZE, 0);    // PROPER ERROR HANDLING
                
                // read incomming message into buffer
                rbuf[readr] = '\0';
                
                // app logic (common)
                bank_processing(bank, rbuf, wbuf);

                // send the reply
                sendr = send(pfd.fd, wbuf, _BANK__BUF_SIZE, 0);  // PROPER ERROR HANDLING
                continue;
            }
        } else {
            printf("error with poll\n");
            goto error;
        } // don't have to handle timeout because timeout is infinite

        printf("uh oh\n");

    } while (1);

    printf("server program terminating\n");

    close(sockfd);
    close(consockfd);
    free(rbuf);
    free(wbuf);

    return BANKSERV_OK;

error:
    printf("there was an error running the server quitting server event loop\n");

    close(sockfd);
    close(consockfd);
    free(rbuf);
    free(wbuf);

    return BANKSERV_ERR;
}

/**
*/
int bank_server_udp(appmode_t* am, bank_t* bank)
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

    char* rbuf = (char*)calloc(_BANK__BUF_SIZE, sizeof(char));
    char* wbuf = (char*)calloc(_BANK__BUF_SIZE, sizeof(char));

    // 1 socket setup
    if (getsockfd_udp(&sockfd) > 0)
        goto error;
    setsockaddr_lb(&saddr, am->port); // sockaddr -> lb:port
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
                readr = recvfrom(sockfd, rbuf, _BANK__BUF_SIZE, 0,
                    (struct sockaddr*)&caddr, &caddr_len);      // PROPER ERROR HANDLING
                
                // read incomming message into buffer
                rbuf[readr] = '\0';

                // app logic (common)
                bank_processing(bank, rbuf, wbuf);
                    
                // send the reply
                sendr = sendto(sockfd, wbuf, _BANK__BUF_SIZE, 0,
                    (struct sockaddr*)&caddr, caddr_len); // PROPER ERROR HANDLING
                continue;
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
    free(wbuf);

    return BANKSERV_OK;

error:
    printf("there was an error running the server quitting server event loop\n");

    close(sockfd);
    free(rbuf);
    free(wbuf);

    return BANKSERV_ERR;
}


int main(int argc, char** argv)
{
    int pport;
    appmode_t mode = { 0 };
    bank_t bank = { 0 };

    bank.balance = 1000;

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
            if (bank_server_tcp(&mode, &bank) != 0)
                goto error;
            break;
        case UDP:
            if (bank_server_udp(&mode, &bank) != 0)
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

