/*
NAME: Ram Goli
ID: 604751659
EMAIL: ramsgoli@gmail.com
 */
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>

// Shell options
#define LOG 'l'
#define ENCRYPT 'e'
#define PORT 'p'

// setup boolean definition
typedef int bool;
#define true 1
#define false 0

// Which port the client wants to connect to
int CLIENT_PORT = -1;

// name of the log file
char *LOG_FILE;

// name of the encryption file name
char *ENCRYPT_FILENAME;

// Buffer used to hold input data and write output data
#define BUFFER_SIZE 256
char buff[BUFFER_SIZE];

// Which HOST we are connecting to. We will be connecting to localhost
const char HOST_NAME[] = "localhost";

// used to restore the terminal settings upon shutdown
struct termios old_term_attrs;

// hold the file descriptor of the socket connection
int sockfd;

struct sockaddr_in serv_addr;
struct hostent *server;

// hold the pollfd structs that hold status information about
// server communication and keyboard communication
struct pollfd poll_fds[2];

void return_error(char *error) {
/*
Print any error message to stderr and exit
 */
    int err = errno;
    fprintf(stderr, "Error from %s call: %s\n", error, strerror(err));
    exit(1);
}

void print_usage() {
/*
Prints out a usage message in case bad arguments are passed to the program
 */
    fprintf(stderr, "%s\n", "Correct usage: ./lab1b-client --port=INTEGER [--encrypt=file_name] [--log=file_name]");
}

void restore_term_attrs() {
/*
Restores terminal attributes from settings in global old_term_attrs struct
 */
    tcsetattr(0, TCSANOW, &old_term_attrs);
}

void term_handler() {
    exit(0);
}

bool read_write(int INPUT_FD, int OUTPUT_FD) {
/*
Read bytes from INPUT_FD and write them one-by-one to OUTPUT_FD
*/
    int read_val = read(INPUT_FD, buff, BUFFER_SIZE);
    if (read_val == -1) {
        return_error("read()");
    }

    for (int i = 0; i < read_val; i++) {
        if (buff[i] == 4) {
            return false;
        } else if (buff[i] == '\r' || buff[i] == '\n') {
            char newline_buff[2] = {'\r', '\n'};
            if (write(OUTPUT_FD, newline_buff, 2) == -1) {
                return_error("write()");
            }
        } else {
            int write_amount = write(OUTPUT_FD, buff+i, 1);
            if (write_amount == -1) {
                return_error("write()");
            }
        }
    }

    return true;
}

void set_non_canonical_input_mode() {
/*
Puts the keyboard (FD 0) in non-canonical input mode with no-echo
 */
    int ret;
    struct termios new_term_attrs;

    ret = tcgetattr(0, &old_term_attrs);
    if (ret == -1) {
        return_error("tcgetattr()");
    }
    ret = atexit(restore_term_attrs);
    if (ret != 0) {
        return_error("atexit()");
    }

    ret = tcgetattr(0, &new_term_attrs);
    if (ret == -1) {
        return_error("tcgetattr()");
    }

    new_term_attrs.c_iflag = ISTRIP;  // only lower seven bits
    new_term_attrs.c_oflag = 0;  // no processing
    new_term_attrs.c_lflag = 0;  // no processing

    ret = tcsetattr(0, TCSANOW, &new_term_attrs);
    if (ret == -1) {
        return_error("tcsetattr()");
    }
}

void start_listening() {
    // enter the INFINITE LOOP
    while(true) {
        int p_ret = poll(poll_fds, 2, 0);
        if (p_ret < 0) {
            return_error("poll()");
        }

        if (poll_fds[0].revents & POLLIN) {
            // from keyboard
            // echo to the terminal and send to the socket
            if (!read_write(poll_fds[0].fd, STDOUT_FILENO)) {
                exit(1);
            }
        }
    }
}

void setup_socket() {
    sockfd = socket(AF_INET, // Address domain of the socket (Internet domain)
                    SOCK_STREAM, // stream socket in which chars are read in a continuous stream
                    0 // the protocol
    );
    if (sockfd < 0) {
        return_error("socket()");
    }

    server = gethostbyname(HOST_NAME);
    if (server == NULL) {
        // server could not be found
        return_error("gethostbyname()");
    }

    // zero out all bytes in the sockadd_in struct
    memset(&serv_addr, 0, sizeof(serv_addr));

    // set address family of the sockaddr_in struct
    serv_addr.sin_family = AF_INET;

    // copy fields in serv_addr from server struct
    memcpy((char *) &serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(CLIENT_PORT);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        return_error("connect()");
    }

    poll_fds[0].fd = STDIN_FILENO;
    poll_fds[1].fd = sockfd;

    poll_fds[0].events = POLLIN;
    poll_fds[1].events = POLLIN;

    signal(SIGTERM, term_handler);
}

int main(int argc, char *argv[]) {
    int c;
    struct option long_options[] = {
        {"log", required_argument, NULL, LOG},
        {"encrypt", required_argument, NULL, ENCRYPT},
        {"port", required_argument, NULL, PORT},
        {0,0,0,0}
    };

    while(true) {
        int option_index = 0;
        c = getopt_long(argc, argv, "l:e:p:", long_options, &option_index);

        if (c == -1) {
            break;
        }

        switch(c) {
            case PORT: {
                CLIENT_PORT = (int)atol(optarg);
                break;
            }
            case LOG: {
                LOG_FILE = optarg;
                break;
            }
            case ENCRYPT: {
                ENCRYPT_FILENAME = optarg;
                break;
            }
            default: {
                print_usage();
                exit(1);
            }
        }
    }

    // check that PORT (mandatory) has been specified
    if (CLIENT_PORT <= 0) {
        print_usage();
        exit(1);
    }

    set_non_canonical_input_mode();
    setup_socket();
    start_listening();

    exit(0);
}
