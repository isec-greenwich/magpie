#include "inet_local.h"


struct LOCAL_SOCKET* generate_free_socket(){
	struct LOCAL_SOCKET* _socket = (LOCAL_SOCKET*)calloc(1, sizeof(struct LOCAL_SOCKET));
	return _socket;
}
void clear_socket(struct LOCAL_SOCKET *_socket){
	close(_socket->socket_no);
	bzero((char *)_socket, sizeof(struct LOCAL_SOCKET));
}
void clear_socket_free(struct LOCAL_SOCKET *_socket){
	clear_socket(_socket);
	free(_socket);
}

int create_tcp_server_connection(struct LOCAL_SOCKET* _socket, int port){
	int err = 0;
	int opt = 1;
	_socket->socket_no = socket(AF_INET, SOCK_STREAM, (int)0);
	if (_socket->socket_no < 0) return -1; // -1 error in issuing the socket
	err = setsockopt( _socket->socket_no, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (err == -1) return -2; // -2 error in setting options
	// The socket was generated using calloc i.e. empty (0's)
	_socket->address.sin_family = AF_INET;
	_socket->address.sin_addr.s_addr = INADDR_ANY;
	_socket->address.sin_port = htons(port);
	// Attempt to bind socket with OS
	if ( bind(_socket->socket_no, (struct sockaddr *) &_socket->address, sizeof(_socket->address)) < 0 ){
		// Error
		return -3; // -3 error in binbing socket
	}
	// Attempt to Listen for Maximum amount of connections
	listen(_socket->socket_no, SOMAXCONN);
	return _socket->socket_no;
}
int create_udp_server_connection(struct LOCAL_SOCKET* _socket, int port){
	int err = 0;
	int opt = 1;
	_socket->socket_no = socket(AF_INET, SOCK_DGRAM, (int)0);
	if (_socket->socket_no < 0) return -1;
	err = setsockopt(_socket->socket_no, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (err == -1) return -2;

	_socket->address.sin_family = AF_INET;
	_socket->address.sin_addr.s_addr = INADDR_ANY;
	_socket->address.sin_port = htons(port);

	err = bind(_socket->socket_no, (struct sockaddr *) &_socket->address, sizeof(_socket->address));
	if (err < 0) return -3;
	return _socket->socket_no;
}
int create_tcp_client_connection(struct LOCAL_SOCKET* _socket, char *ip_addr, int port){
	int err = 0;
	int opt = 1;
	struct hostent *server;
	_socket->socket_no = socket(AF_INET, SOCK_STREAM, (int)0);
	if (_socket->socket_no < 0) return -1;
	// configure the destination
	server = gethostbyname(ip_addr);
	_socket->address.sin_family = AF_INET;
//	memmove(&server->h_addr, &_socket->address.sin_addr.s_addr, server->h_length);
	_socket->address.sin_port = htons(port);
	err = connect(_socket->socket_no, (struct sockaddr*) &_socket->address, sizeof(_socket->address));
	if (err < 0) return -2;
	return _socket->socket_no;

}
int create_udp_client_connection(struct LOCAL_SOCKET* _socket, char *ip_addr, int port){
	int err = 0;
	int opt = 1;
	struct hostent *server;
	_socket->socket_no = socket(AF_INET, SOCK_DGRAM, (int)0);
	if (_socket->socket_no < 0) return -1;

	server = gethostbyname(ip_addr);
	_socket->address.sin_family = AF_INET;
	// memmove
	_socket->address.sin_port = htons(port);
	
	return _socket->socket_no;
}

struct LOCAL_SOCKET* accept_local_tcp_socket(struct LOCAL_SOCKET *_socket){
	struct LOCAL_SOCKET *_socket_cli = generate_free_socket();
	socklen_t addrlen = sizeof(_socket_cli->address);
	_socket_cli->socket_no = accept(_socket->socket_no, (struct sockaddr *) &_socket_cli->address, &addrlen);
	if (_socket_cli < 0){
		// Error
		free(_socket_cli);
		return (struct LOCAL_SOCKET*)-1;
	}
	return _socket_cli;
}

int read_data_from_socket(struct LOCAL_SOCKET* _socket, char *buffer, int buffer_len){
	int len = recv(_socket->socket_no, buffer, buffer_len, 0);
	if (len < 0) return -1; // error occurred
	return len; // number of bytes read
}

int write_data_to_socket(struct LOCAL_SOCKET* _socket, char *buffer, int length){
	int len = send(_socket->socket_no, buffer, length, (int)0);
	if (len < 0) return -1; // error occurred
	return len; // number of bytes sent
}
