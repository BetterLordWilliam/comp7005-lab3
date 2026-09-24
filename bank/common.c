#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include  "common.h"


void print_appmode(appmode_t* am)
{
    printf("ADDR:%d,PROTO:%d,PORT:%d\n", am->addr, am->proto, am->port);
}


int test_port(int port)
{
    if (port <= _BANK__PORT_MIN || port >= _BANK__PORT_MAX)
        return 0;
    return 1;
}


void setsockaddr_lb(struct sockaddr_in* addr, int port)
{
    addr->sin_family        = AF_INET;
    addr->sin_port          = port;
    addr->sin_addr.s_addr   = htonl(INADDR_LOOPBACK); // host to network byte-ordering
    
    // if (inet_pton(AF_INET, _BANK__LOOPBACK_STR, &addr->sin_addr) != 1)
    //    return 1;
}


int getsockfd_tcp(int* fd)
{
    int t;
    while (1) {
        t = socket(AF_INET, SOCK_STREAM, 0);
        if (t > 0)
            break;
        if (t < 0 && errno == EINTR)
            continue;
        if (t < 0)
            return 1;
    }
    *fd = t;
    return 0;
}


int getsockfd_udp(int* fd)
{
    int t;
    while (1) {
        t = socket(AF_INET, SOCK_DGRAM, 0);
        if (t > 0)
            break;
        if (t < 0 && errno == EINTR)
            continue; // try again
        if (t < 0)
            return 1;
        
    }
    *fd = t; 
    return 0;
}


int bindsock(int fd, struct sockaddr* sockaddr, socklen_t addrlen)
{
    int t;
    while (1) {
        t = bind(fd, sockaddr, addrlen);
        if (t == 0)
            break;
        if (t < 0 && errno == EINTR)
            continue;
        return 1;
    }
    return 0;
}

