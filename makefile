.RECIPEPREFIX = >
.DEFAULT_GOAL = build


LOGS        = ./logs
SERVER_IN   = ./logs/server-stdin
SERVER_OUT  = ./logs/server-stdout
SERVER_ERR  = ./logs/server-stderr
CLIENT_IN   = ./logs/client-stdin       # delivering the commands
CLIENT_OUT  = ./logs/client-stdout
CLIENT_ERR  = ./logs/client-err

SRC     = ./bank
BUILD   = ./build


BANK_COMMON_SRC = $(SRC)/common.c
BANK_COMMON_OBJ = $(BUILD)/common.o             # compiled obj of common
BANK_COMMON_LIB = $(BUILD)/common.a             # archived for copy into bins

$(BANK_COMMON_OBJ): $(BANK_COMMON_SRC)
> mkdir -p $(BUILD)
> gcc -c $(BANK_COMMON_SRC) -o $(BANK_COMMON_OBJ)

$(BANK_COMMON_LIB): $(BANK_COMMON_OBJ)
> ar rcs $(BANK_COMMON_LIB) $(BANK_COMMON_OBJ)

BANK_SERVER_SRC = $(SRC)/bank_server.c
BANK_CLIENT_SRC = $(SRC)/bank_client.c
BANK_SERVER     = $(BUILD)/bank-server
BANK_CLIENT     = $(BUILD)/bank-client

$(BANK_SERVER): $(BANK_SERVER_SRC) $(BANK_COMMON_LIB)
> mkdir -p $(BUILD)
> gcc -o $(BUILD)/bank-server $(BANK_SERVER_SRC) $(BANK_COMMON_LIB)

$(BANK_CLIENT): $(BANK_CLIENT_SRC) $(BANK_COMMON_LIB)
> mkdir -p $(BUILD)
> gcc -o $(BUILD)/bank-client $(BANK_CLIENT_SRC) $(BANK_COMMON_LIB)


build: $(BANK_SERVER) $(BANK_CLIENT)


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

