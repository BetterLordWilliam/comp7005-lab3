.RECIPEPREFIX = >
.DEFAULT_GOAL = build

LOGS            = ./logs
SERVER_IN       = ./logs/server-stdin
SERVER_OUT      = ./logs/server-stdout
SERVER_ERR      = ./logs/server-stderr
CLIENT_IN       = ./logs/client-stdin       # delivering the commands
CLIENT_OUT      = ./logs/client-stdout
CLIENT_ERR      = ./logs/client-err

BUILD           = ./build
BANK_SERVER_SRC = ./bank/bank_server.c
BANK_CLIENT_SRC = ./bank/bank_client.c


build-server: $(BANK_SERVER_SRC)
> mkdir -p $(BUILD)
> gcc -o $(BUILD)/bank-server $(BANK_SERVER_SRC)

build-client: $(BANK_CLIENT_SRC)
> mkdir -p $(BUILD)
> gcc -o $(BUILD)/bank-client $(BANK_CLIENT_SRC)

build: build-server build-client


clean:
> rm $(BUILD)/*

clean-logs:
> rm $(LOGS)/*


# will need to implement proto specific running recipes (fine for now)
# i guess I actually don't care about stdin for the server
# and I can just run the client in the foreground (w/ no redirect stdout for client requried)

run-both: build
> mkdir -p $(LOGS)
> ./build/bank-server > $(SERVER_OUT) 2> $(SERVER_ERR) &
> ./build/bank-client 2> $(CLIENT_ERR)

