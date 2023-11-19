#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

// This is an error-handling function that prints error messages
// and exits the program when something goes wrong.

void error(const char *msg) {
    perror(msg);
    exit(1);
}

struct serverConfig { // eu poderia chamar isso de fd?
    char * url;
    char host[100];
    char path [100];
    int port;
    struct hostent * server;
};

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        exit(1);
    }

    struct serverConfig serverConf;

    serverConf.url = argv[1];
    serverConf.port = 80;

    //like a regex
    if (sscanf(serverConf.url, "http://%99[^/]/%99[^\n]", serverConf.host, serverConf.path) != 2) {
        fprintf(stderr, "Invalid URL format.\n");
        exit(1);
    }

    struct hostent *server = gethostbyname(serverConf.host); // struct hostent contains tons of information, including the IP address.

    if (server == NULL) {
        error("Error: Unable to resolve host");
    }

    struct sockaddr_in server_addr;
    memset((char *)&server_addr, 0, sizeof(server_addr)); // cleaning the memory to dont override anything and dont have suck
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length); // copying the value to server_addr to be used after in the connection
    server_addr.sin_port = htons(serverConf.port);

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Error: Unable to open socket");
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("Error: Unable to connect");
    } 

    // Send an HTTP GET request
    char request[300];
    snprintf(request, sizeof(request), "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", serverConf.path, serverConf.host);

    if (write(sockfd, request, strlen(request)) < 0) {
        error("Error: Unable to write to socket");
    }

    // Read and print the response
    char buffer[1024];
    int n;
    while ((n = read(sockfd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    }

    if (n < 0) {
        error("Error: Unable to read from socket");
    }

    // Close the socket
    close(sockfd);

    return 0;
}