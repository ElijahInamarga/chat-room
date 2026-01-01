#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    char message[256] = "Successfully connected";

    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if(socket_fd == -1) {
        printf("Error creating socket\n");
	return -1;
    }

    // server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9002);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int bind_status = bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(bind_status != 0) {
	printf("Error binding server socket to IP and port\n");
	return -1;
    }

    listen(socket_fd, 1);

    int client_socket_fd = accept(socket_fd, NULL, NULL);

    send(client_socket_fd, message, sizeof(message), 0);

    close(socket_fd);
    return 0;
}
