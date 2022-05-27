HEADERS=include
SRC=src

CC=gcc

HFILES=$(shell find $(HEADERS) -name '*.h' | sed 's/^.\///')
FILES=$(shell find $(SRC) -name '*.c' | sed 's/^.\///')

OFILES=$(patsubst %.c,./%.o,$(FILES))
CFLAGS = -Wall -Wextra -pedantic -lpthread -pedantic-errors -O3 -g -std=c11 -D_POSIX_C_SOURCE=200112L $(MYCFLAGS)

%.o: %.c $(HFILES)
	$(CC) -c -o $@ $< $(CFLAGS)

all:$(OFILES)
	$(CC) $(OFILES) $(CFLAGS) -o server


clean:	
	rm -rf $(OFILES)
	
