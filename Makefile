CC = gcc
CFLAGS += -g -std=c99 -Wall -I include
LDFLAGS += -L. -lconn

test_dir=test
src=src
conn=connection
INC=include

lib_ob=$(src)/$(conn)/tcp_sock_handler.o
OBJ=$(test_dir)/test_tcp_sock_handler.o $(test_dir)/simple_client.o 
test_bin=$(test_dir)/test_tcp_sock_handler $(test_dir)/simple_client $(test_dir)/simple_server
test_run=$(test_dir)/test_runner.sh

exe=$(addprefix ./, $(test_bin))

all: $(test_bin)

libconn.so: $(lib_ob)
	$(CC) $(CFLAGS) $< -shared -o $@

test: $(test_bin)
	sh $(test_run) $(exe)

$(OBJ): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

$(OBJ): $(lib_ob)

$(test_bin): %: %.o
	 $(CC) $(CFLAGS) $(lib_ob) $^ -o $@

clean:
	-rm $(lib_ob)
	-rm $(test_bin)
	-rm $(OBJ)

