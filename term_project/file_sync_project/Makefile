CC = gcc
CFLAGS = -Wall -Iinclude
SRC = src/main.c
OBJ = $(SRC:src/%.c=build/%.o)
OUT = file_sync

all: $(OUT)

build/%.o: src/%.c
	@mkdir -p backup build source remote
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(OUT)

clean:
	rm -rf build $(OUT) source remote backup
