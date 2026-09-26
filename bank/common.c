#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
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
    addr->sin_port          = htons(port);
    addr->sin_addr.s_addr   = htonl(INADDR_LOOPBACK); // host to network byte-ordering
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


ssize_t recvfrom_all(int fd, char* buf, ssize_t buf_len,
    struct sockaddr* saddr, socklen_t* saddr_len)
{
    ssize_t received = 0;
    do {
        received = recvfrom(fd, buf, (size_t)buf_len, 0, saddr, saddr_len);
        if (received < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        buf[received] = '\0';
        break;
    } while (1);
    return received;
}


ssize_t sendto_all(int fd, char* buf, ssize_t buf_len,
    struct sockaddr* saddr, socklen_t saddr_len)
{
    ssize_t sent = 0;
    do {
        sent = sendto(fd, buf, (size_t)buf_len, 0, saddr, saddr_len);
        if (sent < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        break; 
    } while (1);
    return sent;
}


ssize_t recv_all(int fd, char* buf, ssize_t buf_len)
{
    ssize_t received = 0;
    do {
        ssize_t n = recv(fd, buf + received, buf_len - 1 - received, 0);
        if (n == 0) {
            return 0;
        }
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        received += n;
        buf[received] = '\0';
        if (strchr(buf, '\n') != NULL)
            break;
    } while (received < buf_len - 1);
    return received;
}


ssize_t send_all(int fd, char* buf, ssize_t buf_len)
{
    ssize_t sent = 0;
    do {
        ssize_t n = send(fd, buf + sent, buf_len - sent, 0);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        sent += n;
    } while (sent < buf_len);
    return sent;
}


int consock(int fd, const struct sockaddr* saddr, socklen_t saddr_len)
{
    int r = 0;
    do {
        r = connect(fd, saddr, saddr_len);
        if (r < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        break;
    } while (1);
    return r;
}


ssize_t readstdin(int fd, char* buf, size_t buf_len)
{
    ssize_t n = 0;
    do {
        n = read(fd, buf, buf_len);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        buf[n] = '\0';
        break;
    } while (1);
    return n;
}

int acceptcon(int fd)
{
    int cfd = 0;
    do {
        cfd = accept(fd, NULL, NULL);
        if (cfd < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        return cfd;
        break;
    } while (1);
}

