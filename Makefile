CC = gcc
test_dir=test
src=src
conn=connection
lib_ob=$(src)/$(conn)/tcp_sock_handler.o
OBJ=$(test_dir)/test_tcp_sock_handler.o $(test_dir)/simple_client.o 
INC=include
CFLAGS = -g -std=c99 -Wall -fPIC
test_bin=$(test_dir)/test_tcp_sock_handler $(test_dir)/simple_clientj
test_run=$(test_dir)/test_runner.sh
exe=$(addprefix ./, $(test_bin))

all: libconn.so

libconn.so: $(lib_ob)
	$(CC) $(CFLAGS) $< -shared -o $@


test: $(test_bin)
	sh $(test_run) $(exe)

$(OBJ): %.o: %.c
	$(CFLAGS) -c $< -I $(INC) -o $@ 

simple_server.o: $(test_dir)/simpe_server.c $(INC)/tcp_sock_handler.h

$(test_bin): %: %.o
	 $(CFLAGS) $(LIB_OBJ) $^ -o $@

clean:
	-rm $(test_bin)
	-rm $(OBJ)
	-rm $(LIB_OBJ)

