#ifndef _BANK__COMMON
#define _BANK__COMMON

#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define _BANK__TCP_PROTO    "tcp"
#define _BANK__UDP_PROTO    "udp"

// [WO] just use the <arpa/inet.h> definitions
// #define _BANK__LOOPBACK_STR "127.0.0.1"
// #define _BANK__LOOPBACK_BIN (0b01111111000000000000000000000001)
// #define _BANK__LOOPBACK_DEC (2130706433)

#define _BANK__PORT_MIN (1024)
#define _BANK__PORT_MAX (65535)

#define _BANK__BUF_SIZE (256)

#define _BANK__REPLY_MSG_PREFIX "OK"
#define _BANK__SERVER_SHUTDOWN_MSG_PREFIX "BYE"
#define _BANK__BALANCE_MSG_PREFIX "BALANCE"
#define _BANK__DEPOSIT_MSG_PREFIX "DEPOSIT"
#define _BANK__WITHDRAW_MSG_PREFIX "WITHDRAW"
#define _BANK__QUIT_MSG "QUIT"


enum protocol {
    TCP,
    UDP
} typedef protocol_t;


struct appmode {
    protocol_t proto;   /** PROTOCOL TL/L4 */
    int addr;           /** IP address (IPv4) */
    int port;           /** intended application port */
} typedef appmode_t;


struct bank {
    int balance;
    int should_quit;
} typedef bank_t;


/** helper for printinf appmode_t structs  */
void print_appmode(appmode_t* am);

/**
validates a port
    given a port number represented by integer `port` determine if the port is
    in the valid range of ports for the application (1024 <= `port` <= 65535)
    
    returns 0 if false  (port is not valid)
    returns 1 if true   (port is valid)
*/
int test_port(int port);

/**
sets sockaddr_in struct fields & assigns address as loopback address.
    uses `inet_pton`
*/
void setsockaddr_lb(struct sockaddr_in* addr, int port);

int getsockfd_tcp(int* fd);
int getsockfd_udp(int* fd);

/**
safely handles the bind syscall
    by safely I really just mean it will retry if the reported `errno` is EINTR
    0 is returned if `bind` returns 0
    & 1 is returned if a value < 0 is returned by `bind`
*/
int bindsock(int fd, struct sockaddr* sockaddr, socklen_t addrlen);

#endif

