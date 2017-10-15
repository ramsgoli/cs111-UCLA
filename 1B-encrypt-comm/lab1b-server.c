/*
NAME: Ram Goli
ID: 604751659
EMAIL: ramsgoli@gmail.com
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>

// setup boolean definition
typedef int bool;
#define true 1
#define false 0

// Shell options
#define PORT 'p'

// Buffer used to hold input data and write output data
#define BUFFER_SIZE 256
char buff[BUFFER_SIZE];

// Which port the client wants to connect to
int SERVER_PORT = -1;

// Which HOST we are connecting to. We will be connecting to localhost
const char HOST_NAME[] = "localhost";

// used to send data and read date from the child process shell
int to_child_pipe[2];
int from_child_pipe[2];
pid_t child_pid = -1;

// The program that will be executed by the child process shell
char execvp_filename[] = "/bin/bash";
char *execvp_argv[2] = {execvp_filename, NULL};

int sockfd, newsockfd;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;

// hold the pollfd structs that hold status information about
// shell and client communication
struct pollfd poll_fds[2];

void print_usage() {
/*
Prints out a usage message in case bad arguments are passed to the program
 */
    fprintf(stderr, "%s\n", "Correct usage: ./lab1b-client --port=INTEGER [--encrypt=file_name] [--log=file_name]");
}

void return_error(char *error) {
/*
Print any error message to stderr and exit
 */
    int err = errno;
    fprintf(stderr, "Error from %s call: %s\n", error, strerror(err));
    exit(1);
}

void process_shutdown() {
    // close all pipes
    close(to_child_pipe[0]);
    close(to_child_pipe[1]);
    close(from_child_pipe[0]);
    close(from_child_pipe[1]);
}

bool read_write(int INPUT_FD, int OUTPUT_FD, bool from_socket) {
/*
Read bytes from INPUT_FD and write them one-by-one to OUTPUT_FD

PARAMS:
    * from_socket: if true, we send input to child process, so
    *   we map <cr> or <lf> to <lf>. Else, we send everything
    *   to the client and let it handle formatting how it wishes
*/
    bool success = true;

    int read_val = read(INPUT_FD, buff, BUFFER_SIZE);
    if (read_val == -1) {
        return_error("read()");
    }

    for (int i = 0; i < read_val; i++) {
        if (buff[i] == 4) {
            // we close the write end of pipe but continue processing the input
            close(to_child_pipe[1]);
            success = false;
        } else if (buff[i] == 3) {
            // ^C
            kill(SIGINT, child_pid);
            return false;
        } else if ((buff[i] == '\r' || buff[i] == '\n') && from_socket) {
            char newline_buff[2] = {'\r', '\n'};
            if (write(OUTPUT_FD, newline_buff+1, 1) == -1) {
                return_error("write()");
            }
        } else {
            int write_amount = write(OUTPUT_FD, buff+i, 1);
            if (write_amount == -1) {
                return_error("write()");
            }
        }
    }

    return success;
}

void handle_connection() {
    const int NUM_FDS = 2;
    const int TIMEOUT = 0;


    while (true) {
        int p_ret = poll(poll_fds, NUM_FDS, TIMEOUT);
        if (p_ret < 0) {
            return_error("poll()");
        }

        if (poll_fds[0].revents & POLLIN) {
            // from client
            // forward to child process
            if (!read_write(poll_fds[0].fd, to_child_pipe[1], true)) {
                exit(0);
            }
        }

        if (poll_fds[1].revents & POLLIN) {
            // from shell
            // forward to client
            if (!read_write(poll_fds[1].fd, newsockfd, false)) {
                exit(0);
            }
        }

        if ((poll_fds[0].revents & (POLLERR | POLLHUP)) ||
            (poll_fds[1].revents & (POLLERR | POLLHUP))) {
                exit(0);
        }
    }

}

void accept_connections() {
    // Hang until we get a connection
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        return_error("accept()");
    }

    if (pipe(to_child_pipe) == -1) {
        return_error("pipe(to_child_pipe)");
    }
    if (pipe(from_child_pipe) == -1) {
        return_error("pipe(from_child_pipe)");
    }
    child_pid = fork();

    if (child_pid > 0) { // parent proce

        // close unused file descriptors
        // we don't read from the to_child_pipe
        // and we don't write to the from_child_pipe
        close(to_child_pipe[0]);
        close(from_child_pipe[1]);

        poll_fds[0].fd = newsockfd;
        poll_fds[1].fd = from_child_pipe[0];

        poll_fds[0].events = POLLIN;
        poll_fds[1].events = POLLIN;

        atexit(process_shutdown);

        handle_connection();
    } else if (child_pid == 0) { // child process
        close(to_child_pipe[1]);
        close(from_child_pipe[0]);

        dup2(to_child_pipe[0], STDIN_FILENO);
        dup2(from_child_pipe[1], STDOUT_FILENO);
        close(to_child_pipe[0]);
        close(from_child_pipe[1]);

        if (execvp(execvp_filename, execvp_argv) == -1) {
            return_error("execvp()");
        }
    } else {
        return_error("fork()");
    }
}

void setup_socket() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return_error("socket()");
    }

    // zero out all bytes in the sockadd_in struct
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        return_error("bind()");
    }
    listen(sockfd,5);

    clilen = sizeof(cli_addr);
}

int main(int argc, char *argv[]) {
    int c;
    struct option long_options[] = {
        {"port", required_argument, NULL, PORT},
        {0,0,0,0}
    };

    while(true) {
        int option_index = 0;
        c = getopt_long(argc, argv, "p:", long_options, &option_index);

        if (c == -1) {
            break;
        }

        switch(c) {
            case PORT: {
                SERVER_PORT = (int)atol(optarg);
                break;
            }
            default: {
                print_usage();
                exit(1);
            }
        }
    }

    // check that PORT (mandatory) has been specified
    if (SERVER_PORT <= 0) {
        print_usage();
        exit(1);
    }

    setup_socket();
    accept_connections();
}
