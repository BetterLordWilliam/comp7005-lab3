#ifndef _BANK__COMMON
#define _BANK__COMMON

#define _BANK__TCP_PROTO "tcp"
#define _BANK__UDP_PROTO "udp"


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

#endif

