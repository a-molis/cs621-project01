# Compression Detection
Developed by Andrew Molis

## Build
The project is written in C with make as the build system. To build the project simply run `make`. 
To clean the project run `make clean`.  

## Development Environment
Development was done on vm slices made by USFCA CS support. The VMs are running AlmaLinux 9.1 (Lime Lynx) x86_64.
The project was not tested with Ubuntu.

## Project Design
The project has two major components.
1. Compression detection with a server and client
2. Compression detection with a client using a server that does not run any code.

### Connection Framework
Basic TCP and UDP application frameworks were designed for this project.
These include wrappers for sockets and functions around sending and receiving data with TCP and UDP. 
These are built as abstractions of using sockets. 
The functions allow for ease of use with creating and using TCP/UDP sockets without having to work with sockets directly.
For example, the functions `tcp_send` and `tcp_recv` send and receive an extra packet with the size information of the data for the second packet.
This allows for sending and receiving data using TCP without knowing the size of the data beforehand. 
The code for these functions is stored in ` src/connection/tcp_sock_handler.c` and `src/connection/udp_sock_handler.c`.
The full documentation on how to use all of these functions are located in `include/tcp_sock_handler.h ` and `include/udp_sock_handler.h`

### Tests
Tests were written while developing the project. Each major feature was built out then tested starting with simple tests for the connection framework. 
As the building blocks of the project were factored system tests were developed alongside the code for part 1 and 2.

The more comprehensive tests like `test/test_part_one` are designed around testing that part 1 works holistically.
Threads are used to simulate different machines. A server and client are spun up in each thread and the code for part 1 is then run.

#### Running the tests
The tests can be run with the test_runner.sh script. Compiled binaries for each test are passed in as arguments for the script.
The script then runs each test and prints out the result of all tests. This requires that the tests binaries be first built.
The tests can be built with `make`. The test script can be run with `make test`. 
In order to test part 2 of the project the test `test/test_part_two` needs to be run with root privileges. 
The command `make test_all` will run the all the tests and `test/test_part_two` as root. 

##### Example output of running `make test`
```
Running Test Suite

         Test ./test/test_simple_client_server PASSED

         Test ./test/test_tcp_send PASSED

         Test ./test/test_simple_udp_conn PASSED

         Test ./test/test_udp_setup PASSED

         Test ./test/test_pre_probe PASSED

         Test ./test/test_udp_probe PASSED

It took 97 ms between packet between packets 0 and 5999 for high entropy data
It took 98 ms between packet between packets 0 and 5999 for high entropy data
Result from server results:
It took 97 ms between packet between packets 0 and 5999 for high entropy data
It took 98 ms between packet between packets 0 and 5999 for high entropy data
Compression detected: False


Results from server:
It took 97 ms between packet between packets 0 and 5999 for high entropy data
It took 98 ms between packet between packets 0 and 5999 for high entropy data
Compression detected: False
         Test ./test/test_part_one PASSED

  -------------------------------------------------------
  | Results: SUCCESS (7 tests, 7 successes, 0 failures) |
  -------------------------------------------------------
```

### Constants
Defaults and constants are stored in `include/constants.h`.

### Config
The configuration file (config) is used to store configurable data for the project. 
The config file `myconfig.json` is an example config that was used for testing the project during development. 

The JSON parser cJSON is used for the project to parse the config file.
The source code for this project was added to ext/cJSON in this project at `ext/cJSON/cJSON.c` and `ext/cJSON/cJSON.h`.
The original source code for cJSON is stored at `https://github.com/DaveGamble/cJSON`.

The project wraps up cJSON functions and parses them in the file `src/config/config.c`. 
Documentation for each function is stored in `include/config.h`. 

Example of creating a `CONFIG`. 

```c
char *config_path = "test/test_config.json";
char buf[100];
CONFIG config = get_config (config_path, buf);

// ...do something with the config

config_destroy (config);
```

The api get_config  opens the config from the config_path. Convert the config json into a CONFIG struct. 
The string representation of the json is stored in the buf. The string representation can be sent as data over TCP to the server.
Then the server can create a `CONFIG` from the json representation of the string using the function `config_new`.
The pointer `CONFIG` is a pointer to the `CONFIG_DATA` struct. 
Structs for the project are typedef as pointers to a struct to further abstract the types. 

All the fields that are included in the project spec are in the CONFIG_DATA struct. 
The optional fields are not required to be in the json. If an optional field is not found in the json the default value will be used from `include/constants.h`.   
Additionally, the following fields were added as optional fields.  

The config file has the following field's
1. `client_ip` This is the IPv4 address of the client. 
2. `raw_packet_size` This is the max size of a raw packet used for part 2.
3. `recv_device` This is the network device name of the network card used in part 2 for setting promiscuous mode for the raw socket.
4. `tcp_src_syn_port` This is the port used for the source port for sending the UDP packet train in part 2.

### Part 1
Part 1 detects network compression using a server and a client model. 
The main function for part 1 is stored in `src/client_server_compdetect/compdetect_client.c` for the client and `src/client_server_compdetect/compdetect_server.c` for the server.

#### Running Part 1
To run part 1 `make` must first be run. The config should be updated with the correct IPv4 address for the server as well as port configurations that the user wants to use. 
The project spec was followed to determine which config variable should be used for each part of the project.

First the server should be started with `src/client_server_compdetect/compdetect_server 12062` ran at the root of the project code.
This port number should match the `tcp_probing_port` in the config.

To start the client run `src/client_server_compdetect/compdetect_client myconfig.json` ran at the root of the project code. 
Then wait for the results to display on the client. 

#### Part 1 and 2 Code
The business logic code for part 1 and 2 are found in `src/client_server_compdetect/comp_utils.c`. With the public functions documented in `include/comp_utils.h`.

### Part 2
Part 2 detects network compression using a client. A server must be running but does not need to have any specific program running associated with this project. 
The server must have three unused ports that are not open that can be using for probing and sending UDP packets. 
These ports are stored in the `myconfig.json` as `udp_dest_port`, `tcp_dest_head_syn_port`, and `tcp_dest_tail_syn_port`. 
Both the `client_ip` and `server_ip` IPv4 address should be populated in the config.
The network card device/interface name must be present in the config json as `recv_device` or else the default name will be used from `include/constants.h` under `DEFAULT_DEVICE`.

#### Running Part 2
Once the configuration file json is set up correctly run `make` to build the project. 
The run `sudo src/client_server_compdetect/compdetect myconfig.json` to run the code for part 2. Then wait for the results.

### random_file
The random file contains data to be used for sending high entropy data.

## Pcap files
The pcap files are located at `tcpdumps/`. Part 1 client and server pcaps are located at `tcpdumps/part1-client.pcap` and `part1-server.pcap`.
The part 2 pcap files is located at `tcpdumps/part2-compdetect.pcap`.


## Project Completion
All parts of the project are complete.