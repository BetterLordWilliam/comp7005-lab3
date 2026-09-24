#ifndef _BANK__COMMON
#define _BANK__COMMON

#define _BANK__TCP_PROTO "tcp"
#define _BANK__UDP_PROTO "udp"

#define _BANK__PORT_MIN (1024)
#define _BANK__PORT_MAX (65535)


enum protocol {
    TCP,
    UDP
} typedef protocol_t;


struct appmode {
    protocol_t proto;   /** PROTOCOL TL/L4 */
    int addr;           /** IP address (IPv4) */
    int port;           /** intended application port */
} typedef appmode_t;


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

#endif

