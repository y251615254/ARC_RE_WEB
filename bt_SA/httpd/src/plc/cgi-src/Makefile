CC = gcc

OBJ = test.cgi

DEBUG_CFLAGS= -g
COMMON_CFLAGS= $(CFLAGS) $(DEBUG_FLAGS) -Wall -Werror -rdynamic

all: $(OBJ)

test.cgi: test.c
	$(CC) $(COMMON_CFLAGS) $(LDFLAGS)   $< -o $@

install:
	$(CP) ./test.cgi $(HTTP_DIR)/cgi-bin/

clean:
	rm -rf *.o $(OBJ)


