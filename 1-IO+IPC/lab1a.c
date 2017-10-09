/*
NAME: Ram Goli
EMAIL: ramsgoli@gmail.com
ID: 604751659
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <getopt.h>
#include <poll.h>

#define SHELL 's'

typedef int bool;
#define true 1
#define false 0

// used to restore the terminal settings upon shutdown
struct termios old_term_attrs;

// size of the buffer we will use to read input from
const int BUFFER_SIZE = 128;
char *buff;
char *mapped_buff;
bool shell = false;

// used to send data and read date from the child process shell
int to_child_pipe[2];
int from_child_pipe[2];
pid_t child_pid = -1;

// The program that will be executed by the child process shell
char execvp_filename[] = "/bin/bash";
char *execvp_argv[2] = {execvp_filename, NULL};

struct pollfd keyboard_input;
struct pollfd pipe_input;

int timeout = 0;


void print_usage() {
    fprintf(stderr, "%s\n", "Correct usage: ./lab1a [--shell]");
}
void return_error(char *error) {
    int err = errno;
    fprintf(stderr, "Error from %s call: %s\n", error, strerror(err));
    exit(1);
}

void restore_term_attrs() {
    tcsetattr(0, TCSANOW, &old_term_attrs);
}

void set_non_canonical_input_mode() {
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

void map_to_linefeed(int size) {
    free(mapped_buff);
    mapped_buff = (char *)malloc(BUFFER_SIZE);
    int i = 0;

    while (i < size) {
        if (buff[i] == '\r' || buff[i] == '\n') {
            mapped_buff[i] = '\n';
        }
        else {
            mapped_buff[i] = buff[i];
        }
        i++;
    }
}

void map_to_newline(int size) {
    free(mapped_buff);
    mapped_buff = (char *)malloc(BUFFER_SIZE);
    int i = 0;

    while (i < size) {
        if (buff[i] == '\r' || buff[i] == '\n') {
            mapped_buff[i] = '\r';
            i++;
            mapped_buff[i] = '\n';
        }
        else {
            mapped_buff[i] = buff[i];
        }
        i++;
    }
}

int main(int argc, char *argv[]) {
    int c;

    struct option long_options[] = {
        {"shell", no_argument, NULL, SHELL},
    };

    while (1) {

        int option_index = 0;

        c = getopt_long(argc, argv, "s", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch(c) {
            case SHELL: {
                shell = true;
                break;
            }
            default: {
                print_usage();
                exit(1);
            }
        }
    }

    // used to hold results of poll function
    struct pollfd poll_fds[2] = {
        keyboard_input,
        pipe_input
    };
    int TIMEOUT = 0;
    int nfds = 2;

    if (shell) {
        if (pipe(to_child_pipe) == -1) {
            return_error("pipe(to_child_pipe)");
        }
        if (pipe(from_child_pipe) == -1) {
            return_error("pipe(from_child_pipe)");
        }
        child_pid = fork();

        if (child_pid > 0) { // parent process

            // close unused file descriptors
            // we don't read from the to_child_pipe
            // and we don't write to the from_child_pipe
            close(to_child_pipe[0]);
            close(from_child_pipe[1]);

            keyboard_input.fd = STDIN_FILENO;
            pipe_input.fd = from_child_pipe[0];
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

        // the pollfds should only listen for input or error events
        for (int i = 0; i < 2; i++) {
            poll_fds[i].events = POLLIN | POLLHUP | POLLERR;
        }
    }

    set_non_canonical_input_mode();

    // start performing I/O
    buff = (char*)malloc(BUFFER_SIZE);
    int read_val;
    bool should_break = false;
    int num_newlines = 0;

    while (1) {
        if (shell) {
            // we will poll to wait for input
            int pret = poll(poll_fds, nfds, TIMEOUT);
            if (pret == -1) {
                return_error("poll()");
            }
            for (int i = 0; i < 2; i++) {
                if (poll_fds[i].revents == POLLIN) {
                    // data to read on this file descriptor
                    read_val = read(poll_fds[i].fd, buff, BUFFER_SIZE);
                    for (int i = 0; i < read_val; i++) {
                        if (buff[i] == '\r' || buff[i] == '\n') {
                            num_newlines++;
                        }
                        if (buff[i] == 4) {
                            // EOF
                            should_break = true;
                            // TODO: FIX
                            exit(1);
                            break;
                        }
                        if (buff[i] == 3) {
                            // SIGINT. Kill the process
                            kill(getpid(), SIGINT);
                        }

                    }
                    if (num_newlines) {
                        map_to_newline(read_val);
                        int write_amount = write(1, mapped_buff, read_val + num_newlines);
                        if (write_amount == -1) {
                            return_error("write(1)");
                        }

                        map_to_linefeed(read_val);
                        write_amount = write(to_child_pipe[1], mapped_buff, read_val + num_newlines);
                        if (write_amount == -1) {
                            return_error("write(2)");
                        }
                    } else {
                        int write_amount = write(1, buff, read_val);
                        if (write_amount == -1) {
                            return_error("write(3)");
                        }
                        write_amount = write(to_child_pipe[1], buff, read_val);
                        if (write_amount == -1) {
                            return_error("write(4)");
                        }
                    }
                }
            }
        } else {
            // no shell. Do everything normally
            read_val = read(0, buff, sizeof(buff));
            for (int i = 0; i < read_val; i++) {
                if (buff[i] == '\r' || buff[i] == '\n') {
                    num_newlines++;
                }
                if (buff[i] == 4) {
                    // EOF
                    should_break = true;
                    break;
                }
                if (buff[i] == 3) {
                    // SIGINT. Kill the process
                    kill(getpid(), SIGINT);
                }

            }
            if (should_break) {
                break;
            }
            // map any delimeter characters (like '\r' or '\n' to '\r\n'), if any, and write
            if (num_newlines) {
                map_to_newline(read_val + num_newlines);

                int write_amount = write(1, mapped_buff, read_val + num_newlines);
                if (write_amount == -1) {
                    return_error("write()");
                }
            } else {
                int write_amount = write(1, buff, read_val);
                if (write_amount == -1) {
                    return_error("write()");
                }
            }
        }

    }

    // clean up our buffers
    free(buff);
    free(mapped_buff);
    exit(EXIT_SUCCESS);
}
