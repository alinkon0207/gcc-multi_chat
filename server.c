#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5000

int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
    fd_set readfds;

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("waiting for clients...\n");

    // Add server socket to read set
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);

    // Accept incoming connections and handle messages
    while (1) {
        // Wait for events on read set
        if (select(server_fd+1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // Check if server socket has event pending
        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                           (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("New connection from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Add new client socket to read set
            FD_SET(new_socket, &readfds);
        }

        // Check if client sockets have events pending
        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &readfds)) {
                if ((valread = read(i, buffer, 1024)) == 0) {
                    // Client disconnected
                    printf("Client disconnected\n");
                    close(i);
                    FD_CLR(i, &readfds);
                } else if (valread > 0) {
                    // Handle incoming message
                    printf("read %d bytes from client\n", valread);
                    printf("Message from client: %s\n", buffer);
                    send(i, hello, strlen(hello), 0);
                    memset(buffer, 0, sizeof(buffer));
                }
            }
        }
    }

    return 0;
}
