# COMP7005 Week3 Lab Report
Will Otterbein, A01372608

## Summary

This report describes my implementation of the week3 bank server lab.


## Obtaining a Program Binary

This section describes how to compile the project.


## Implementation

This section describes how the programs (client & server) were implemented 
for use with TCP protocol & UDP protocol.

### Client (main)

![client-main](./screenshots/client-main.c)

```c
int main(int argc, char** argv)
{
    int pport;

    // initialize application mode all fields to 0
    appmode_t mode = { 0 };

    // determine proto & port from program arguments
    // we need 3 arguments to this program
    // be strict, reject more or less
    if (argc != 3) {
        printf("%s incorrect number of arguments\n", MESSAGE_PREFIX);
        goto error;
    }
    // arg1 protocol type
    if (strcmp(_BANK__TCP_PROTO, argv[1]) == 0 ) {
        mode.proto = TCP;
    } else if (strcmp(_BANK__UDP_PROTO, argv[1]) == 0) {
        mode.proto = UDP;
    } else {
        printf("%s unknown protocol\n", MESSAGE_PREFIX);
        goto error;
    }
    // arg2 port
    if ((pport = atoi(argv[2])) != 0 && test_port(pport))  {
        mode.port = pport;
    } else {
        printf("%s failed to parse port to integer\n", MESSAGE_PREFIX);
        goto error;
    }

    // sanity check
    print_appmode(&mode);

    // protocol specific socket setup
    switch (mode.proto) {
        case TCP:
            if (bank_client_tcp(&mode) > 0)
                goto error;
            break;

        case UDP:
            if (bank_client_udp(&mode) > 0)
                goto error;
            break;

        default:
            goto error; // should be an impossibility to get here
    }

    return 0;

error:
    printf("error running client program exiting\n");

    return 1;
}
```

The job of the main method in the client program is to parse the command line
arguments into the `mode` instance of `appmode_t` struct & kick start either
the `TCP` or `UDP` client event loop.

Argument parsing is strict, so the exact count (3) must be met.

Argument 0 is the program name, this is ignored.

Argument 1 is the protocol, this is matched against the `_BANK__TCP_PROTO` and
`_BANK__UDP_PROTO` macro strings, matches are recorded the `TCP` & `UDP` enumeration
values (0, & 1 respectively) & written to `mode.proto`.

Argument 2 is the port, this is parsed from ASCII to integer & the `test_port`
helper function of `common.c` is used to determine if the port is within the
valid range of ports, defined as follows:

```c
#define _BANK__PORT_MIN (1024)
#define _BANK__PORT_MAX (65535)
```

If `mode.proto` is evaluated to be `TCP`, then `bank_server_tcp` is invoked.

If `mode.proto` is evaluated to be `UDP`, then `bank_server_udp` is invoked.

### Server (main)


![server-main](./screenshots/server-main.c)

```c
int main(int argc, char** argv)
{
    int pport;
    appmode_t mode = { 0 };
    bank_t bank = { 0 };

    bank.balance = 1000;

    // parse arguments
    // determine proto & port from program arguments
    // we need 3 arguments to this program
    // be strict, reject more or less

    if (argc != 3) {
        printf("%s incorrect number of arguments\n", MESSAGE_PREFIX);
        goto error;
    }
    // arg1 protocol type
    if (strcmp(_BANK__TCP_PROTO, argv[1]) == 0 ) {
        mode.proto = TCP;
    } else if (strcmp(_BANK__UDP_PROTO, argv[1]) == 0) {
        mode.proto = UDP;
    } else {
        printf("%s unknown protocol\n", MESSAGE_PREFIX);
        goto error;
    }
    // arg2 port
    if ((pport = atoi(argv[2])) != 0 && test_port(pport))  {
        mode.port = pport;
    } else {
        printf("%s failed to parse port to integer\n", MESSAGE_PREFIX);
        goto error;
    }

    print_appmode(&mode);

    // start the bank server according to the proto argument
    printf("starting server w/ proto %d\n", mode.proto);

    switch (mode.proto) {
        case TCP:
            if (bank_server_tcp(&mode, &bank) != 0)
                goto error;
            break;
        case UDP:
            if (bank_server_udp(&mode, &bank) != 0)
                goto error;
            break;
        default:
            printf("critical server error server attempted start w/ non TCP or UDP protocol\n");
            goto error;
    }

    printf("server finished execution\n");

    return 0;

error:
    printf("error running server program exiting\n");

    return 1;
}
```

The job of the main method in the server program is largely the same as the client program,
to parse the command line arguments into the `mode` instance of `appmode_t`
struct & kick start either the `TCP` or `UDP` server event loop.

Unlike the client program, the `main` method of the server program
also initialized the values of the `bank_t` struct type, the field `balance` is
set to 1000 & the flag `should_quit` is zeroed (false).

If `mode.proto` is evaluated to be `TCP`, then `bank_server_tcp` is invoked.

If `mode.proto` is evaluated to be `UDP`, then `bank_server_udp` is invoked.

### TCP Client

![tcp-client-setup-error](./screenshots/tcp-client-setup-error.png)

![tcp-client-successful-poll-handling](./screenshots/tcp-client-successful-poll-handling.png)

### TCP Server

![tcp-server-setup-error](./screenshots/tcp-server-setup-error.png)

![tcp-server-successful-poll-handling](./screenshots/tcp-server-successful-poll-handling.png)

### UDP Client

![udp-client-setup-error](./screenshots/udp-client-setup-error.png)

![udp-client-successful-poll-handling](./screenshots/udp-client-successful-poll-handling.png)

### UDP Server

![udp-server-setup-error](./screenshots/udp-server-setup-error.png)

![udp-server-successful-poll-handling](./screenshots/udp-server-successful-poll-handling.png)

## Test Results

Here are the test results w/ the TCP & UDP versions of the program.

Results of TCP tests:
![tcpprototestresults](./screenshots/tcp-proto-test-results.png)

Results of UDP tests:
![udpprototestresults](./screenshots/udp-proto-test-results.png)


