CC = gcc
CFLAGS += -g -std=c99 -I include -include include/config.h -include include/comp_utils.h -include include/udp_sock_handler.h -include include/tcp_sock_handler.h -I ext/cJSON -pthread

test_dir=test
src=src
conn=connection
INC=include

lib_ob=$(src)/$(conn)/tcp_sock_handler.o $(src)/$(conn)/udp_socket_handler.o ext/cJSON/cJSON.o src/config/config.o src/client_server_compdetect/comp_utils.o
OBJ=$(test_dir)/test_simple_client_server.o $(test_dir)/test_tcp_send.o test/test_simple_udp_conn.o test/test_udp_setup.o test/test_pre_probe.o test/test_udp_probe.o test/test_part_one.o test/test_part_two.o
test_run=$(test_dir)/test_runner.sh
test_files=$(test_dir)/test_simple_client_server $(test_dir)/test_tcp_send test/test_simple_udp_conn test/test_udp_setup test/test_pre_probe test/test_udp_probe test/test_part_one
root_test_files=test/test_part_two $(test_files)
exe=$(addprefix ./, $(test_files))
bin=src/client_server_compdetect/compdetect_client src/client_server_compdetect/compdetect_server src/client_server_compdetect/compdetect

all: $(test_bin) $(bin) $(root_test_files)

test: $(test_files)
	sh $(test_run) $(exe)

test_all:
	sudo sh $(test_run) $(root_test_files)

$(OBJ): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ): $(lib_ob)

$(bin): %: %.c
	$(CC) $(CFLAGS) $(lib_ob) $^ -o $@

$(test_bin): %: %.o
	 $(CC) $(CFLAGS) $(lib_ob) $^ -o $@

$(root_test_files): %: %.o
	$(CC) $(CFLAGS) $(lib_ob) $^ -o $@

clean:
	-rm $(lib_ob)
	-rm $(test_bin)
	-rm $(OBJ)
	-rm $(root_test_files)
	-rm $(bin)

