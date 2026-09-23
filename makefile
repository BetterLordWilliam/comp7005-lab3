.RECIPEPREFIX = >
.DEFAULT_GOAL = build

LOGS        = ./logs
SERVER_IN   = ./logs/server-stdin
SERVER_OUT  = ./logs/server-stdout
SERVER_ERR  = ./logs/server-stderr
CLIENT_IN   = ./logs/client-stdin       # delivering the commands
CLIENT_OUT  = ./logs/client-stdout
CLIENT_ERR  = ./logs/client-err

BUILD           = ./build
BANK_SERVER_SRC = ./bank/bank_server.c
BANK_CLIENT_SRC = ./bank/bank_client.c
BANK_COMMON_SRC = ./bank/common.c

BANK_COMMON_LIB = $(BUILD)/common.so


$(BANK_COMMON_LIB): $(BANK_COMMON_SRC)
> mkdir -p $(BUILD)
> gcc -shared -fPIC $(BANK_COMMON_SRC) -o $(BANK_COMMON_LIB)

build-server: $(BANK_SERVER_SRC) $(BANK_COMMON_LIB)
> mkdir -p $(BUILD)
> gcc -o $(BUILD)/bank-server $(BANK_COMMON_LIB) $(BANK_SERVER_SRC) -L$(BUILD) -Wl,-rpath,''$$ORIGIN

build-client: $(BANK_CLIENT_SRC) $(COMMON_LIB)
> mkdir -p $(BUILD)
> gcc -o $(BUILD)/bank-client $(BANK_COMMON_LIB) $(BANK_CLIENT_SRC) -L$(BUILD) -Wl,-rpath,'$$ORIGIN'

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

