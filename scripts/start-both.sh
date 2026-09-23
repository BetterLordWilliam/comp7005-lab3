#!/usr/bin/bash

# run this script from the project root

ERR="\e[31m"
INF="\e[32m"
CL="\e[0m"

EXPECTED_SERVER_BINARY=./build/bank-server
EXPECTED_CLIENT_BINARY=./build/bank-client

if [ ! -f $EXPECTED_SERVER_BINARY ]; then
    printf "($ERR)server binary missing($CL)\n"
    exit 1
fi

printf ""

if [ ! -f $EXPECTED_CLIENT_BINARY ]; then
    printf "($ERR)client binary missing($CL)\n"
    exit 1
fi

printf ""

