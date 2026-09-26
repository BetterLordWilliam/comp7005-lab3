#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
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
void bank_processing(bank_t* bank, const char* request, char* response) {
    int t = -1;

    if (strncmp(_BANK__BALANCE_MSG_PREFIX,
            request, strlen(_BANK__BALANCE_MSG_PREFIX)) == 0) {

        sprintf(response, _BANK__REPLY_MSG_PREFIX " " _BANK__BALANCE_MSG_PREFIX " %d\n",
            bank->balance);

    } else if (strncmp(_BANK__DEPOSIT_MSG_PREFIX,
            request, strlen(_BANK__DEPOSIT_MSG_PREFIX)) == 0) {

        sscanf(request, _BANK__DEPOSIT_MSG_PREFIX "  %d", &t);
        if (t > 0)
            bank->balance += t;
        sprintf(response, _BANK__REPLY_MSG_PREFIX " " _BANK__BALANCE_MSG_PREFIX " %d\n",
            bank->balance);

    } else if (strncmp(_BANK__WITHDRAW_MSG_PREFIX,
            request, strlen(_BANK__WITHDRAW_MSG_PREFIX)) == 0) {

        sscanf(request, _BANK__WITHDRAW_MSG_PREFIX " %d", &t);
        if (t > 0 && t < bank->balance)
            bank->balance -= t;
        sprintf(response, _BANK__REPLY_MSG_PREFIX " " _BANK__BALANCE_MSG_PREFIX " %d\n",
            bank->balance);

    } else if (strncmp(_BANK__QUIT_MSG,
            request, strlen(_BANK__QUIT_MSG)) == 0) {

        sprintf(response, _BANK__SERVER_SHUTDOWN_MSG_PREFIX "\n");
        bank->should_quit = 1;

    } else {
        printf("unknown message type received\n");
    }
}


enum ret_bankserver {
    BANKSERV_OK,
    BANKSERV_ERR
};


/**
*/
int bank_server_tcp(appmode_t* am, bank_t* bank)
{
    int sockfd = -1;
    int consockfd = -1;

    int listenr;
    int acceptr;
    int pollr;
    int sendr;
    int readr;

    struct sockaddr_in saddr    = { 0 };
    struct pollfd pfd           = { 0 };

    char* rbuf = (char*)calloc(_BANK__BUF_SIZE, sizeof(char));
    char* wbuf = (char*)calloc(_BANK__BUF_SIZE, sizeof(char));
    
    // tcp socket setup
    if (getsockfd_tcp(&sockfd) > 0)
        goto error;
    setsockaddr_lb(&saddr, am->port); // sockaddr -> lb:port
    if (bindsock(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)))
        goto error;

    // tcp listen for connections w/ queue size 1
    listenr = listen(sockfd, 1); // double check connection queue size
    if (listenr != 0)
        goto error;

    // tcp server loop 
    do {
        // tcp connection acceptance (1 client at a time)
        consockfd = accept(sockfd, NULL, NULL); // block me until connection is made, returns new connection fd
        if (consockfd < 0) continue; // retry if connection fails
        
        // tcp connection poll loop
        pfd.fd      = consockfd;
        pfd.events  = POLLIN;
        pfd.revents = 0;
        
        do {
            pollr = poll(&pfd, 1, -1);

            // handle poll stuff
            if (pollr > 0) {
                if (pfd.revents & ( POLLNVAL | POLLERR ))  {
                    goto error;
                } else if (pfd.revents & ( POLLHUP )) {
                    break;
                } else {
                    memset(rbuf, 0, _BANK__BUF_SIZE);
                    memset(wbuf, 0, _BANK__BUF_SIZE);

                    // read incomming message into buffer
                    // tcp implications
                    readr = recv_all(pfd.fd, rbuf, (ssize_t)_BANK__BUF_SIZE);
                    if (readr < 0) {
                        printf("error reading from client socket closing connection\n");
                        break;
                    } else if (readr == 0) {
                        printf("server read EOF from socket connection connection closed\n");
                        break;
                    }
                    // check if the command is valid, ignore it if it isnt
                    if (strchr(rbuf, '\n') == NULL) {
                        printf("invalid command\n");
                        continue;
                    }

                    // app logic (common)
                    bank_processing(bank, rbuf, wbuf);

                    // send the reply
                    sendr = send_all(pfd.fd, wbuf, (ssize_t)strlen(wbuf));
                    if (sendr < 0) {
                        printf("error writing to client socket closing connection\n");
                        break;
                    }
                    // time to end the server
                    if (bank->should_quit) {
                        printf("QUIT command received closing connection & terminating server\n");
                        break;
                    }

                    continue;
                }
            } else {
                printf("error with poll\n");
                goto error;
            } // don't have to handle timeout because timeout is infinite

            printf("uh oh\n");

        } while (1);

        close(consockfd);
        
        // time to end the server 
        if (bank->should_quit)
            break;

    } while (1);

    printf("server program terminating\n");
    
    if (sockfd >= 0)
        close(sockfd);
    if (consockfd >= 0)
        close(consockfd);
    free(rbuf);
    free(wbuf);

    return BANKSERV_OK;

error:
    printf("there was an error running the server quitting server event loop\n");
    
    if (sockfd >= 0)
        close(sockfd);
    if (consockfd >= 0)
        close(consockfd);
    free(rbuf);
    free(wbuf);

    return BANKSERV_ERR;
}

/**
*/
int bank_server_udp(appmode_t* am, bank_t* bank)
{
    int sockfd = -1;

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

    // udp socket setup
    if (getsockfd_udp(&sockfd) > 0)
        goto error;
    setsockaddr_lb(&saddr, am->port); // sockaddr -> lb:port
    if (bindsock(sockfd, (struct sockaddr*)&saddr, sizeof(saddr)))
        goto error;
    
    // socket poll loop
    pfd.fd      = sockfd;
    pfd.events  = POLLIN;
    pfd.revents = 0;

    do {
        pollr = poll(&pfd, 1, -1);

        // handle poll stuff
        if (pollr > 0) {
            if (pfd.revents & ( POLLNVAL | POLLERR ))  {        // POLLHUP not raised (UDP therefore no connection)
                goto error;

            } else {
                memset(rbuf, 0, _BANK__BUF_SIZE);
                memset(wbuf, 0, _BANK__BUF_SIZE);

                // readr = recvfrom(sockfd, rbuf, _BANK__BUF_SIZE, 0,
                //     (struct sockaddr*)&caddr, &caddr_len);
                // if (readr == -1) {
                //    if (errno == EINTR) continue;
                //    break;
                // }

                readr = recvfrom_all(pfd.fd, rbuf, _BANK__BUF_SIZE,
                    (struct sockaddr*)&caddr, &caddr_len);
                if (readr < 0) {
                    printf("there was an error reading the clients message\n");
                    continue;
                }
                if (strchr(rbuf, '\n') == NULL) {
                    printf("invalid command\n");
                    continue;
                }
                
                // app logic (common)
                bank_processing(bank, rbuf, wbuf);

                // send the reply
                // sendr = sendto(sockfd, wbuf, strlen(wbuf), 0,
                //    (struct sockaddr*)&caddr, caddr_len);
                sendr = sendto_all(pfd.fd, wbuf, strlen(wbuf),
                    (struct sockaddr*)&caddr, caddr_len);
                if (sendr < 0) {
                    printf("there was an error sending reply message to the client\n");
                    continue;
                }

                // time to end the server
                if (bank->should_quit) {
                    printf("quit command recieved terminating server\n");
                    break;
                }

                continue;
            }

        } else {
            printf("error with poll\n");
            goto error;
        } // don't have to handle timeout because timeout is infinite

        printf("uh oh\n");

    } while (1);

    printf("server program terminating\n");

    if (sockfd >= 0) 
        close(sockfd);
    free(rbuf);
    free(wbuf);

    return BANKSERV_OK;

error:
    printf("there was an error running the server quitting server event loop\n");

    if (sockfd >= 0)
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

    // start the bank server according to the proto argument
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

