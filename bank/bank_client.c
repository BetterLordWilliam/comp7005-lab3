#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "common.h"


#define MESSAGE_PREFIX "[CLIENT]"


enum ret_bankclient {
    BANKCLIENT_OK,
    BANKCLIENT_ERR
} typedef ret_bankclient_t;


int bank_client_tcp(appmode_t* mode)
{
    int sockfd = -1;
    int connectr;
    int pollr;
    int readr;
    int sendr;
    int recvr;

    struct sockaddr_in saddr = { 0 };
    struct pollfd pfd = { 0 };

    char* rbuf = (char*)calloc(_BANK__BUF_SIZE, sizeof(char));
    char* wbuf = (char*)calloc(_BANK__BUF_SIZE, sizeof(char));

    // create TCP socket
    if (getsockfd_tcp(&sockfd) > 0)
        goto error;
    setsockaddr_lb(&saddr, mode->port); // sockaddr -> lb:port
    
    // connect to server with TCP
    connectr = connect(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
    if (connectr < 0)
        goto error;

    pfd.fd      = STDIN_FILENO; // stdin because we are waiting for user input (which is the message)
    pfd.events  = POLLIN;
    pfd.revents = 0;
    
    printf("connection to server established entering poll loop.\n");
    do {
        pollr = poll(&pfd, 1, -1); // poll on stdin (messages)
        
        if (pollr > 0) {
            if (pfd.revents & ( POLLNVAL | POLLERR | POLLHUP )) {
                goto error; 

            } else if (pfd.revents & ( POLLIN )) {
                memset(rbuf, 0, _BANK__BUF_SIZE);
                memset(wbuf, 0, _BANK__BUF_SIZE);

                readr = read(pfd.fd, rbuf, _BANK__BUF_SIZE - 1);
                if (readr < 0)
                    continue; // failed to read for this command
                if (readr == 0) {
                    break;
                }
                rbuf[readr] = '\0';

                sendr = send(sockfd, rbuf, readr, 0); // TCP implications
                if (sendr < 0) {
                }

                recvr = recv(sockfd, wbuf, _BANK__BUF_SIZE - 1, 0);
                if (recvr < 0) {
                    if (errno == EINTR) continue;
                    goto error;
                }
                if (recvr == 0) {
                    printf("server closed the connection\n");
                    break;
                }
                wbuf[recvr] = '\0';
                char* newline = strchr(wbuf, '\n');
                if (newline != NULL) {
                    *newline = '\0';
                } else {
                    printf("invalid reply\n");
                    continue;
                }
                printf("%s\n", wbuf); // reply from the server

                // if the server replied with the shutdown message
                // break out of the event loop
                if (strncmp(_BANK__SERVER_SHUTDOWN_MSG_PREFIX, wbuf, strlen(_BANK__SERVER_SHUTDOWN_MSG_PREFIX)) == 0)
                    break;
    
                continue;
            }

        } else {
            if (errno == EINTR)
                continue;
            printf("error with poll\n");
            goto error;
        } // don't have to handle timeout because timeout is infinite
        printf("uh oh\n");
    } while (1);

    if (sockfd > 0)
        close(sockfd);

    free(rbuf);
    free(wbuf);

    return 0;

error:
    if (sockfd > 0)
        close(sockfd);

    free(rbuf);
    free(wbuf);

    return 1;
}


int bank_client_udp(appmode_t* mode)
{
    int sockfd = -1;
    int connectr;
    int pollr;
    int readr;
    int sendr;
    int recvr;

    struct sockaddr_in saddr = { 0 };
    struct pollfd pfd = { 0 };

    char* rbuf = (char*)calloc(_BANK__BUF_SIZE, sizeof(char));
    char* wbuf = (char*)calloc(_BANK__BUF_SIZE, sizeof(char));

    // create UDP socket
    if (getsockfd_udp(&sockfd) > 0)
        goto error;
    setsockaddr_lb(&saddr, mode->port); // sockaddr -> lb:port
    
    // using connect with UDP ensures that I dont always need to 
    // pass the address around
    connectr = connect(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
    if (connectr < 0)
        goto error;

    pfd.fd      = STDIN_FILENO; // stdin because we are waiting for user input (which is the message)
    pfd.events  = POLLIN;
    pfd.revents = 0;
    
    printf("connection to server established entering poll loop.\n");

    do {
        pollr = poll(&pfd, 1, -1); // poll on stdin (messages)
        
        if (pollr > 0) {
            if (pfd.revents & ( POLLNVAL | POLLERR | POLLHUP )) {
                goto error; 

            } else if (pfd.revents & ( POLLIN )) {
                memset(rbuf, 0, _BANK__BUF_SIZE);
                memset(wbuf, 0, _BANK__BUF_SIZE);

                readr = read(pfd.fd, rbuf, _BANK__BUF_SIZE - 1);
                if (readr < 0)
                    continue; // failed to read for this command
                if (readr == 0) {
                    break;
                }
                rbuf[readr] = '\0';

                sendr = send(sockfd, rbuf, readr, 0); // TCP implications
                if (sendr < 0) {
                }

                recvr = recv(sockfd, wbuf, _BANK__BUF_SIZE - 1, 0);
                if (recvr < 0) {
                    if (errno == EINTR) continue;
                    goto error;
                }
                if (recvr == 0) {
                    printf("server closed the connection\n");
                    break;
                }
                wbuf[recvr] = '\0';
                char* newline = strchr(wbuf, '\n');
                if (newline != NULL) {
                    *newline = '\0';
                } else {
                    printf("invalid reply\n");
                    continue;
                }
                printf("%s\n", wbuf); // reply from the server

                // if the server replied with the shutdown message
                // break out of the event loop
                if (strncmp(_BANK__SERVER_SHUTDOWN_MSG_PREFIX, wbuf, strlen(_BANK__SERVER_SHUTDOWN_MSG_PREFIX)) == 0)
                    break;
    
                continue;
            }

        } else {
            if (errno == EINTR)
                continue;
            printf("error with poll\n");
            goto error;
        } // don't have to handle timeout because timeout is infinite

        printf("uh oh\n");

    } while (1);

    printf("client program terminating\n");

    if (sockfd > 0) 
        close(sockfd);

    free(rbuf);
    free(wbuf);

    return 0;

error:
    if (sockfd > 0)
        close(sockfd);

    free(rbuf);
    free(wbuf);

    return 1;
}


int main(int argc, char** argv)
{
    int pport;
    int done;
    
    // client state (just track if we are done or not) 
    done = 0;

    // initialize application mode all fields to 0
    appmode_t mode = { 0 };

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
    
    // sanity check
    print_appmode(&mode);

    // protocol specific socket setup
    switch (mode.proto) {
        case TCP:
            if (bank_client_tcp(&mode) > 0)
                goto error;
            break;
            
        case UDP:
            if (bank_client_udp(&mode) > 0)
                goto error;
            break;

        default:
            goto error; // should be an impossibility to get here
    }

    return 0;

error:
    printf("error running client program exiting\n");

    return 1;
}
