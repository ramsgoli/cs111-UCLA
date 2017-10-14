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

// used to restore the terminal settings upon shutdown
struct termios old_term_attrs;

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
Restores terminal attributes
 */
    tcsetattr(0, TCSANOW, &old_term_attrs);
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
    if (CLIENT_PORT == -1 || CLIENT_PORT == 0) {
        print_usage();
        exit(1);
    }

    set_non_canonical_input_mode();

    while(true) {
        bool success = read_write(STDIN_FILENO, STDOUT_FILENO);
        if (!success) {
            break;
        }
    }
    exit(0);
}
