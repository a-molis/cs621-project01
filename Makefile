CC = gcc
CFLAGS += -g -std=c99 -Wall -I include -include include/config.h -I ext/cJSON

test_dir=test
src=src
conn=connection
INC=include

lib_ob=$(src)/$(conn)/tcp_sock_handler.o $(src)/$(conn)/udp_socket_handler.o ext/cJSON/cJSON.o src/config/config.o
OBJ=$(test_dir)/simple_client.o $(test_dir)/simple_server.o $(test_dir)/test_simple_client_server.o $(test_dir)/test_tcp_send.o test/test_simple_udp_conn.o test/test_udp_setup.o
test_bin=$(test_dir)/simple_client $(test_dir)/simple_server
test_run=$(test_dir)/test_runner.sh
test_files=$(test_dir)/test_simple_client_server $(test_dir)/test_tcp_send test/test_simple_udp_conn test/test_udp_setup
exe=$(addprefix ./, $(test_files))
bin=src/client_server_compdetect/compdetect_client src/client_server_compdetect/compdetect_server

all: $(test_bin) $(test_files) $(bin)

simple_server: $(test_dir)/simple_server

test: $(test_files)
	sh $(test_run) $(exe)

$(OBJ): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ): $(lib_ob)

$(bin): %: %.c
	$(CC) $(CFLAGS) $(lib_ob) $^ -o $@

$(test_bin): %: %.o
	 $(CC) $(CFLAGS) $(lib_ob) $^ -o $@

$(test_files): %: %.o
	$(CC) $(CFLAGS) $(lib_ob) $^ -o $@

clean:
	-rm $(lib_ob)
	-rm $(test_bin)
	-rm $(OBJ)
	-rm $(test_files)
	-rm $(bin)

