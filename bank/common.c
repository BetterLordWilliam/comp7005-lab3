#include <stdio.h>


#include  "common.h"


void print_appmode(appmode_t* am)
{
    printf("ADDR:%d,PROTO:%d,PORT:%d\n", am->addr, am->proto, am->port);
}


