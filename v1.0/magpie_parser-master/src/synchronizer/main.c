#include <stdio.h>
#include <time.h>
#include "inet_local.h"

time_t timestamp;
int cli_connected = 0;

LOCAL_SOCKET *cli[10];

void send_activate_to_all(){
        int i = 0;
        PDU d_str;
        d_str.timestamp = (int)timestamp;
        d_str.command = 1;
        while(i < cli_connected){
                write_data_to_socket(cli[i], (char*)&d_str, sizeof(d_str));
                i++;
        }
	printf("All activate: %d", timestamp);
}

void sync_all(int seconds){
	int i = 0;
	PDU d_str;
	d_str.timestamp = seconds;
	d_str.command = 16;
	while(i < cli_connected){
		write_data_to_socket(cli[i], (char*)&d_str, sizeof(d_str));
		i++;
	}
	printf("Sync all sent! Time: %d\n", seconds);
}
void main(int argc, char *argv[]){
	printf("Create a socket\n");
	LOCAL_SOCKET *sock = generate_free_socket();
	if (create_tcp_server_connection(sock, 31337) < 0){
		printf("Failed to create a socket!\n");
		return;
	}
//	printf("The TCP server has been created!\n");

//	printf("Awaiting the bottleneck\n");
	cli[0] = accept_local_tcp_socket(sock); cli_connected++;
//	printf("Received a first connection\n");
	fflush(stdout);
	PDU buffer;
//	read_data_from_socket(cli[0], buffer, sizeof(buffer));
	while(cli_connected != 5){ // waiting for all nodes to connect
		cli[cli_connected] = accept_local_tcp_socket(sock); cli_connected++;
	}
	printf("Collected all clients\n");
	timestamp = time(NULL);
	sync_all((int)timestamp);
	printf("Synchronizing!\n");
	while(1){
		read_data_from_socket(cli[0], (char*)&buffer, sizeof(buffer)); // read from the command node
		timestamp = time(NULL);
		if ((char)buffer.command == 2){ // ready
			send_activate_to_all(); // to master as well
			printf("signalling\n");
		}
		fflush(stdout);
	}

	return;
}
