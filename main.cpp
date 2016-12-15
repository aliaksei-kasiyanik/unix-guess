#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <inttypes.h>
#include <netinet/in.h>

#define MAX 1000000000
#define MIN 0

void PrintUsage(const char* prog) {
    fprintf(stderr, "=== number guessing client ===\n");
    fprintf(stderr, "Usage: %s UNIX_SOCKET_PATH \n\n", prog);
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        PrintUsage(argv[0]);
        return 2;
    }

    int fd;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return 1;
    }

    const char* socketPath = argv[1];

    struct sockaddr_un local;
    local.sun_family = AF_UNIX;
    if (strlen(socketPath) >= sizeof(local.sun_path)) {
        fprintf(stderr, "path '%s' is too long for UNIX domain socket\n", socketPath);
        return 1;
    }

    strcpy(local.sun_path, socketPath);
    socklen_t localLen = sizeof(local);

    fprintf(stderr, "Trying to connect...\n");

    if (connect(fd, (struct sockaddr *) &local, localLen) == -1) {
        perror("connect");
        close(fd);
        exit(1);
    }

    fprintf(stderr, "Connected.\n");

    char response;
    ssize_t response_size;

    uint32_t b_e_guess;
    uint32_t guess;

    uint32_t left = MIN;
    uint32_t right = MAX;

    for(;;) {
        guess = left + (right - left) / 2;
        b_e_guess = htonl(guess);

        fprintf(stderr, "My guess is %d \n", guess);

        if (send(fd, (const char*) &b_e_guess, sizeof(guess), 0) == -1) {
            perror("send");
            close(fd);
            exit(1);
        }

        response_size = recv(fd, &response, sizeof(response), 0);
        if (response_size == 1) {
            fprintf(stderr, "The response is %c \n", response);
            switch (response) {
                case '>':
                    left = guess + 1;
                    break;
                case '<':
                    right = guess;
                    break;
                case '=':
                    fprintf(stdout, "%d", guess);
                    close(fd);
                    exit(0);
                default:
                    printf("\n  not respond correctly...");
                    break;
            }
        } else {
            if (response_size < 0) {
                perror("recv");
            } else {
                fprintf(stderr, "Server closed connection\n");
            }
            close(fd);
            exit(1);
        }
    }
}