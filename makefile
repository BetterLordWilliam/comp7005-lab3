.RECIPEPREFIX = >
.DEFAULT_GOAL = build

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
> rm ./build/*

