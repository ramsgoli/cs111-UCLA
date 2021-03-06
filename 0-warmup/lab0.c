/*
 * NAME: Ram Goli
 * EMAIL: ramsgoli@gmail.com
 * ID: 604751659
 */
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define INPUT 'i'
#define OUTPUT 'o'
#define SEG_FAULT 's'
#define CATCH 'c'

typedef int bool;
#define true 1
#define false 0

void print_usage() {
    fprintf(stderr, "%s\n", "Correct usage: ./lab0 [--segfault] [--catch] [--input=FILE_NAME] [--output=FILE_NAME]");
}

void handleInput(char *file_name) {
    int ifd = open(file_name, O_RDONLY);
    if (ifd >= 0) {
        close(0);
        dup(ifd);
        close(ifd);
    } else {
        int err = errno;
        fprintf(stderr, "%s: %s, %s\n", "Error caused by --input option", strerror(err), file_name);
        exit(2);
    }
}

void handleOutput(char *file_name) {
    int ofd = creat(file_name, 0666);
    if (ofd >= 0) {
        close(1);
        dup(ofd);
        close(ofd);
    } else {
        int err = errno;
        fprintf(stderr, "%s: %s, %s\n", "Error caused by --output option", strerror(err), file_name);
        exit(3);
    }
}

void handleSegFault() {
    int *bad_pointer = NULL;
    *bad_pointer = 1;
}

void handleCatch(int signo) {
    if (signo == SIGSEGV) {
        fprintf(stderr, "%s\n", "Segmentation fault. Attempting to access NULL pointer.");
        exit(4);
    }
}

void copy_stdin_to_stdout() {
	char input[1];
	int read_val = read(0, &input, sizeof(input));
	while (1) {
		if (read_val == 0)
			break;
		if (read_val == -1) {
			int err = errno;
			fprintf(stderr, "Error reading from stdin: %s\n", strerror(err));
			exit(2);
		}
		int write_amount = write(1, &input, sizeof(input));
		if (write_amount == -1) {
			int err = errno;
			fprintf(stderr, "Error writing to stdout: %s\n", strerror(err));
			exit(3);
		}
		read_val = read(0, &input, sizeof(input));
    }
    return;
}
int main(int argc, char *argv[]) {
    
    int c;

    bool seg_fault = false;
    bool catch = false;
    char *input = NULL;
    char *output = NULL;

    // long options
    struct option long_options[] = {
        {"input", required_argument, NULL, INPUT},
        {"output", required_argument, NULL, OUTPUT},
        {"segfault", no_argument, NULL, SEG_FAULT},
        {"catch", no_argument, NULL, CATCH},
    };

    while(1) {
        int option_index = 0;

        c = getopt_long(argc, argv, "i:o:sc", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch(c) {
            case INPUT: {
                input = optarg;
                break;
            }
            case OUTPUT: {
                output = optarg;
                break;
            }
            case SEG_FAULT: {
                seg_fault = true;
                break;
            }
            case CATCH: {
                catch = true;
                break;
            }
            default: {
                print_usage();
                exit(1);
            }
        }
    }
    if (input != NULL) {
        handleInput(input);
    }
    if (output != NULL) {
        handleOutput(output);
    }
    if (catch) {
        if (signal(SIGSEGV, handleCatch) == SIG_ERR) {
            fprintf(stderr, "%s\n", "Can't catch SIGSEV");
        }
    }
    if (seg_fault) {
        handleSegFault();
    }

    copy_stdin_to_stdout();
    exit(EXIT_SUCCESS);
}

