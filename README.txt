

The JSON parser cJSON is used for the project to parse the config file.
The source code for this project was added to ext/cJSON in this project.
https://github.com/DaveGamble/cJSON

The config file has the following field's
client_ip
raw_packet_size


This was tested on vm slices made by USFCA CS support. The VMs are running AlmaLinux 9.1 (Lime Lynx) x86_64. On this version of AlmaLinux pcap.h is installed with the following command.
sudo yum install libpcap-devel