/*
NAME: Ram Goli
ID: 604751659
EMAIL: ramsgoli@gmail.com
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

// Which port the client wants to connect to
int CLIENT_PORT = -1;

// Which HOST we are connecting to. We will be connecting to localhost
const char HOST_NAME[] = "localhost";

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
                CLIENT_PORT = (int)atol(optarg);
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
}
