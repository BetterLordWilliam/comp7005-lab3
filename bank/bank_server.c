#include <stdio.h>

#include "common.h"


#define MESSAGE_PREFIX "[SERVER]"


int main(int argc, char** argv)
{
    printf("%s starting w/ num arguments: %d\n", MESSAGE_PREFIX, argc);
    return 0;

error:
    printf("error running server program exiting\n");
    return 1;
}

