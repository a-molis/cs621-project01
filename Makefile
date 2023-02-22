CFLAGS = -g -std=c99 
CC = gcc
test_dir=test
OBJ=$(test_dir)/test_tcp_sock_handler.o
INC=include
test_bin=$(test_dir)/test_tcp_sock_handler
test_run=$(test_dir)/test_runner.sh
exe=$(addprefix ./, $(test_bin))

all: $(test_bin)

test: $(test_bin)
	sh $(test_run) $(exe)

$(OBJ): %.o: %.c
	$(CC) $(CFLAGS) -c $< -I $(INC) -o $@ 

$(test_bin): %: %.o
	$(CC) $(CFLAGS) $^ -o $@



clean:
	-rm $(test_bin)
	-rm $(OBJ)
