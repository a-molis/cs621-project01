CFLAGS = -g -std=c99 
CC = gcc
OBJ=tcp_sock_handler.o
INC=include
test_dir=test
bin=test/test_tcp_sock_handler

all: $(bin)

$(OBJ): src/%.o: src/%.c
	$(CC) $(CFLAGS) -I $(INC) -o $@ $<

$(bin): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	-rm $(bin)
	-rm $(OBJ)
