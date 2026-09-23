#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "common.h"


#define MESSAGE_PREFIX "[CLIENT]"

int main(int argc, char** argv)
{
    int pport;

    // initialize application mode all fields to 0
    appmode_t mode = { 0 };

    printf("%s starting w/ num arguments: %d\n", MESSAGE_PREFIX, argc);

    // determine proto & port from program arguments
    // we need 3 arguments to this program
    // be strict, reject more or less

    if (argc != 3)
        goto error;

    // printf("%s arg0: %s\n", MESSAGE_PREFIX, argv[0]);
    // printf("%s arg1: %s\n", MESSAGE_PREFIX, argv[1]);
    // printf("%s arg2: %s\n", MESSAGE_PREFIX, argv[2]);


    // arg1 protocol type
    if (strcmp(_BANK__TCP_PROTO, argv[1]) == 0 ) {
        mode.proto = TCP;

    } else if (strcmp(_BANK__UDP_PROTO, argv[1]) == 0) {
        mode.proto = UDP;

    } else {
        printf("%s unknown protocol\n", MESSAGE_PREFIX);
        goto error;
    }

    // arg2 port process to short?
    if ((pport = atoi(argv[2])) != 0)  {
        mode.port = (short)pport;
    } else {
        printf("%s failed to parse port to integer\n", MESSAGE_PREFIX);
    }
    
    
    // sanity check
    print_appmode(&mode);


    return 0;

error:
    printf("error running client program exiting\n");
    return 1;
}
