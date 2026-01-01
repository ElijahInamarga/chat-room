#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    char server_response[256];

    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if(socket_fd == -1) {
        printf("Error creating socket\n");
        return -1;
    }

    // destination address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9002);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int connect_status = connect(socket_fd, (struct sockaddr *)&server_addr,
                                 sizeof(server_addr));
    if(connect_status == -1) {
        printf("Error connecting to server\n");
        return -1;
    }

    recv(socket_fd, server_response, sizeof(server_response), 0);
    printf("Server response: %s\n", server_response);

    close(socket_fd);

    return 0;
}
