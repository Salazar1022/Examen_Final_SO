CC := gcc
CFLAGS := -Wall -Wextra -O2 -std=c17 -pthread
LDFLAGS := -pthread

SRC := src/main.c src/cli.c src/fs.c src/worker.c src/pipeline.c \
       src/rle.c src/lzw.c src/vigenere.c src/feistel.c src/crc32.c \
       src/header.c src/util.c

OBJ := $(SRC:.c=.o)
BIN := gsea

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean
# End of Makefile