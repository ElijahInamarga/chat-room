#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 256
#define PORT_NUM    8080
#define BACKLOG_LEN 3
#define MAX_CLIENTS 3

/* only used for singleclient to server interation
 * currently not in use
 * IGNORE!
 */
#define NUM_FDS 2 //
int start_session(int socketfd)
{
    // stdin and socket
    struct pollfd fds[NUM_FDS] = {{0, POLLIN, 0}, {socketfd, POLLIN, 0}};

    // 0b 0000 0000 0000 0001 == data available
    for(;;) {
        poll(fds, NUM_FDS, -1);
        char buffer[BUFFER_SIZE] = {0};

        // server to client
        if(fds[0].revents & POLLIN) {
            int bytes_read = read(0, buffer, sizeof(buffer) - 1);
            if(bytes_read > 0) {
                buffer[bytes_read] = '\0';
            }

            // ignore empty inputs
            if(bytes_read == 1 && (buffer[0] == '\n' || buffer[0] == '\r')) {
                continue;
            }

            send(socketfd, buffer, bytes_read, 0);
        }

        // client to server
        if(fds[1].revents & POLLIN) {
            int bytes_read = recv(socketfd, buffer, sizeof(buffer) - 1, 0);
            if(bytes_read == 0) {
                printf("Client has left\n");
                return 0;
            }

            if(bytes_read > 0) {
                buffer[bytes_read] = '\0';
            }

            printf("Client: %s", buffer);
        }
    }

    return 0;
}

void close_session(int socketfd)
{
    close(socketfd);
}

int server_sesh(int listener_socketfd)
{
    int curr_fds = 1;

    struct pollfd fds[MAX_CLIENTS];
    fds[0].fd = listener_socketfd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    for(;;) {
        poll(fds, curr_fds, -1);

        for(int i = 0; i < curr_fds; ++i) {
            char buffer[BUFFER_SIZE] = {0};
            buffer[BUFFER_SIZE - 1] = '\0';

            if(!(fds[i].revents & POLLIN)) {
                continue;
            }

            if(fds[i].fd == listener_socketfd) {
                if(curr_fds >= MAX_CLIENTS) {
                    printf("Server rejected connection\n");

                    // remove client from OS queue
                    int temp_fd = accept(fds[i].fd, NULL, NULL);
                    close(temp_fd);

                    continue;
                }

                int socketfd = accept(fds[i].fd, NULL, NULL);
                if(socketfd == -1) {
                    perror("Could not accept new client\n");
                    continue;
                }

                // add to poll file descriptor list
                fds[curr_fds].fd = socketfd;
                fds[curr_fds].events = POLLIN;
                fds[curr_fds].revents = 0;
                curr_fds++;
		printf("A client has joined the server\n");
            }

            if(fds[i].fd != listener_socketfd) {
                int bytes_read = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);

                if(bytes_read == 0) {
                    printf("A client has left\n");
                    close(fds[i].fd);
                    fds[i] = fds[curr_fds - 1]; // move clients forward
                    fds[curr_fds - 1].fd = -1;  // invalidate duplicate client
                    curr_fds--;
                    i--;
                    continue;
                }

                if(bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                }

		printf("Chat: %s", buffer);

                // send to all clients
                // ignore listener socket and sender socket
                for(int j = 1; j < curr_fds; ++j) {
                    if(fds[j].fd != fds[i].fd) {
                        send(fds[j].fd, buffer, bytes_read, 0);
                    }
                }
            }
        }
    }

    return 0;
}

int start_server()
{
    int socketfd;
    socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if(socketfd == -1) {
        perror("Error creating passive socket\n");
        return -1;
    }

    // server info
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUM);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // allow immediate port reusability
    int opt = 1;
    if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error setting port reusability\n");
    }

    int bind_status =
        bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if(bind_status == -1) {
        perror("Error binding passive socket to IP and port\n");
        return -1;
    }

    listen(socketfd, BACKLOG_LEN);
    printf("Server initiated\n\n");
    return socketfd;
}

void close_server(int passive_socketfd)
{
    close(passive_socketfd);
}

int main()
{
    // start server
    int listener_socketfd = start_server();
    if(listener_socketfd == -1) {
        printf("Could not start server\n");
        return -1;
    }

    // begin session with client
    server_sesh(listener_socketfd);

    close_server(listener_socketfd);

    return 0;
}
