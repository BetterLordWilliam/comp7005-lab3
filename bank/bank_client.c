#include <stdio.h>

#define MESSAGE_PREFIX "CLIENT"

int main(int argc, char** argv)
{
    printf("%s starting w/ num arguments: %d\n", MESSAGE_PREFIX, argc);
    return 0;

error:
    return 1;
}
