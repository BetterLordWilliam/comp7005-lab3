#ifndef _BANK__COMMON
#define _BANK__COMMON

#define _BANK__TCP_PROTO "tcp"
#define _BANK__UDP_PROTO "udp"

extern const int TCP;
extern const int UDP;

struct appmode {
    int addr;   /** IP address (IPv4) */
    int proto;  /** PROTOCOL TL/L4 */
    int port;   /** intended application port */
} typedef appmode_t;


/** helper for printinf appmode_t structs  */
void print_appmode(appmode_t* am);

#endif

