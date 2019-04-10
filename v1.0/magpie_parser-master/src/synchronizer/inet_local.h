#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define IN_BUFFER_LEN  8192 // 8 KB @todo optimize size
#define OUT_BUFFER_LEN 8192 // 8 KB @todo optimize size
#define MAX_CLIENTS    5    // 5 clients allowed

typedef struct LOCAL_SOCKET{
	int socket_no;
	socklen_t frame_len;
	struct sockaddr_in address;
}LOCAL_SOCKET;

typedef struct PDU{
        unsigned int command;
        unsigned int timestamp;
}PDU;

// Prototype functions

struct LOCAL_SOCKET* generate_free_socket(); // generate an empty (0'ed) socket structure
void clear_socket(struct LOCAL_SOCKET *_socket); // close and zero socket structure
void clear_socket_free(struct LOCAL_SOCKET *_socket); // close, zero and free socket structure
int create_tcp_server_connection(struct LOCAL_SOCKET* _socket, int port); // create tcp server connection with provided socket returns socket id or errors -1,-2,-3
int create_udp_server_connection(struct LOCAL_SOCKET* _socket, int port); // create udp server connection with provided socket, returns socket id, -1 as error
int create_tcp_client_connection(struct LOCAL_SOCKET* _socket, char *ip_addr, int port); // creates tcp client connection, providing host/ip and the port number
int create_udp_client_connection(struct LOCAL_SOCKET* _socket, char *ip_addr, int port); // creates udp client connection with provided socket structure, host/ip address and the port number
struct LOCAL_SOCKET* accept_local_tcp_socket(struct LOCAL_SOCKET *_socket); // accept connection, if no connection -1 is returned
int read_data_from_socket(struct LOCAL_SOCKET* _socket, char *buffer, int buffer_len); // read data from socket, providing buffer and the max length, return positive values (number of bytes received), -1 if error
int write_data_to_socket(struct LOCAL_SOCKET* _socket, char *buffer, int length); // write data to specified socket, providing socket structure, buffer and the length of data, returns number of bytes sent, or -1 if error

