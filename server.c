/**
 * Author: Diego Papier-Meister
 * This is the server side of my HTTP Server that I built from scratch.
 * It was my introductory project to C and I am really proud of how it turned out!
 * Refer to the README file for any instructions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define PORT 4221

int main() {

	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	
	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;
	
	// Initializes the socket
	// AF_INET (IPv4) is the domain, SOCK_STREAM (TCP) is the type, internet protocol == 0
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}
	
	// Config socket
	struct sockaddr_in server_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(PORT),
									 .sin_addr = { htonl(INADDR_ANY) },
									};
	
	// Makes sure that the server can bind the socket to the address and port number in server_addr
	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}
	
	// Server into passive mode, waits for client to try and connect
	// Max pending connections: connection_backlog
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	
	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);
	
	/**
	 * Extracts first connection request in queue from server_fd, creates a new connected socket, 
	 * & returns a file descriptor referring to the socket.
	 * Connection is established.
	*/
	int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
	printf("Client connected\n");

	/**
	 * recv() gets the data on the client_fd socket and stores it in the readBuffer buffer.
	 * If successful, returns the length of the datagram in bytes,
	 * otherwise returns -1.
	 * 
	 * A buffer is the temporary data storage area where we can use it to read the path that the user is trying to get to.
	 */
	char readBuffer[1024];
	int bytesRecieved = recv(client_fd, readBuffer, sizeof(readBuffer), 0);

	if (bytesRecieved == -1) {
		printf("Recieving error: %s \n", strerror(errno));
		return 1;
	}

	// Extract the path == GET /something/idk HTTP/1.1...
	char *reqPath = strtok(readBuffer, " ");
	reqPath = strtok(NULL, " ");

	char* content = strtok(NULL, "");

	int bytesSent;
	
	/**
	 * Taking the path that you enter and making endpoints
	 * strcmp() is a comparing method pretty straightforward
	 * 
	 */
	if (strcmp(reqPath, "/") == 0) {
		char *response_200 = "HTTP/1.1 200 OK\r\n\r\n";
		bytesSent = send(client_fd, response_200, strlen(response_200), 0);
	}
	else if (strncmp(reqPath, "/echo/", 6) == 0) {
		reqPath = strtok(reqPath, "/");
		reqPath = strtok(NULL, "");

		int contentLength = strlen(reqPath);
		char response[512];

		sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", contentLength, reqPath);
		printf("Echoing: %s\n", response);
		bytesSent = send(client_fd, response, strlen(response), 0);
	}
	else {
		char *response_404 = "HTTP/1.1 404 Not Found\r\n\r\n";
		bytesSent = send(client_fd, response_404, strlen(response_404), 0);
	}

	if(bytesSent < 0) {
		printf("Send failed");
		return 1;
	}


	close(server_fd);

	return 0;
}