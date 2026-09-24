#include <stdio.h>


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


