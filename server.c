#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#include "threadpool.h"

#define PORT 8081

void handle_connection(void *arg)
{
    int clientfd = *(int *)arg;
    while(1);
}

int
main(int argc, char *argv[])
{
    threadpool *connectionTP = threadpool_create(10, 10);
    fd_set readfds; FD_ZERO(&readfds);

    int server_fd, new_fd;
    struct sockaddr_in address, new_address;
    socklen_t address_len, new_address_len;


    // Create a socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("[ERROR] socket");
        exit(EXIT_FAILURE);
    }

    int maxfd = server_fd + 1;
    FD_SET(server_fd, &readfds);
    // Set server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));

    // Bind the socket to the server address
    if (bind(server_fd, (const struct sockaddr *)&address, sizeof(address)) != 0) {
        perror("[ERROR] bind");
        exit(EXIT_FAILURE);
    }

    // Listen for new connections
    if (listen(server_fd, 10) != 0) {
        perror("[ERROR] listen");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        fd_set tempfds = readfds;
        select(maxfd, &tempfds, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &tempfds)) {
            if ((new_fd = accept(server_fd, (struct sockaddr *)&new_address, (socklen_t *)&new_address_len)) == 0) {
                perror("[ERROR] accept");
                exit(EXIT_FAILURE);
            }
            task handle_client;
            handle_client.function = handle_connection;
            handle_client.argument = (void *)&new_fd;
            add_task(connectionTP, 1, handle_client);
            printf("[SERVER] New Client Connected\n"); 
        }
    }

    return 0;
}