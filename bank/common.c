#include <stdio.h>


#include  "common.h"


const int TCP = 1;
const int UDP = 2;


void print_appmode(appmode_t* am)
{
    printf("ADDR:%d,PROTO:%d,PORT:%d\n", am->addr, am->proto, am->port);
}


