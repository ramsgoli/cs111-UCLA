
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
#include <sys/wait.h>
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
const int BUFFER_SIZE = 2048;
char buff[2048];
bool shell = false;

// used to send data and read date from the child process shell
int to_child_pipe[2];
int from_child_pipe[2];
pid_t child_pid = -1;

// The program that will be executed by the child process shell
char execvp_filename[] = "/bin/bash";
char *execvp_argv[2] = {execvp_filename, NULL};

struct pollfd poll_fds[2];

int timeout = 0;

void print_usage() {
    fprintf(stderr, "%s\n", "Correct usage: ./lab1a [--shell]");
}

void return_error(char *error) {
    int err = errno;
    fprintf(stderr, "Error from %s call: %s\n", error, strerror(err));
    exit(1);
}

void process_shutdown() {
    // continue listening for any remaining input from child pipe
    while (poll_fds[1].revents & POLLIN) {
        if ((poll_fds[0].revents & (POLLERR | POLLHUP)) ||
            (poll_fds[0].revents & (POLLERR | POLLHUP))) {
                break;
            }

        // finish reading any input on child pipe
        int read_val = read(from_child_pipe[0], buff, BUFFER_SIZE);
        if (read_val < 0) {
            break;
        }

        for (int i = 0; i < read_val; i++) {
            if (buff[i] == '\n') {
                char newline_buff[2] = {'\r', '\n'};
                if (write(STDOUT_FILENO, newline_buff, 2) < 0) {
                    return_error("write()");
                        }
            } else {
                if (write(STDOUT_FILENO, buff+i, 1) < 0) {
                    return_error("write()");
                }
            }
        }
    }

    // close all pipes
    close(to_child_pipe[0]);
    close(to_child_pipe[1]);
    close(from_child_pipe[0]);
    close(from_child_pipe[1]);

    int status;
    pid_t w = waitpid(child_pid, &status, 0);
    if (w == -1) {
        return_error("waitpid()");
    }

    if (WIFEXITED(status)) {
        fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", (status & 0x7f), (status & 0xff00) >> 8);
    }
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



void fork_shell_process() {
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

        poll_fds[0].fd = STDIN_FILENO;
        poll_fds[1].fd = from_child_pipe[0];

        poll_fds[0].events = POLLIN;
        poll_fds[1].events = POLLIN;

        atexit(process_shutdown);

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

void handle_sigpipe() {
    exit(0);
}

void shell_mode() {
    signal(SIGPIPE, handle_sigpipe);
    fork_shell_process();

    // enter main I/O Loop
    while (true) {
        int p_ret = poll(poll_fds, 2, 0);
        if (p_ret < 0) {
            return_error("poll()");
        }

        // check for appropriate events in each poll struct
        bool should_exit = false;
        if (poll_fds[0].revents & POLLIN) {
            // from keyboard
            // we echo to stdout and send to child
            int read_val = read(STDIN_FILENO, buff, BUFFER_SIZE);
            if (read_val < 0) {
                return_error("read()");
            }
            for (int i = 0; i < read_val; i++) {
                if (buff[i] == 3) {
                    kill(child_pid, SIGINT);
                } else if (buff[i] == 4) {
                    close(to_child_pipe[1]);
                    // so we continue to process any additional data in the pipe
                    should_exit = true;
                } else if (buff[i] == '\r' || buff[i] == '\n') {
                    char newline_buff[2] = {'\r', '\n'};
                    // we echo as newline but send to the shell a linefeed
                    if (write(STDOUT_FILENO, newline_buff, 2) < 0) {
                        return_error("write(1)");
                    }
                    if (write(to_child_pipe[1], newline_buff+1, 1) < 0) {
                        return_error("write(2)");
                    }
                } else {
                    if (write(STDOUT_FILENO, buff+i, 1) < 0) {
                        return_error("write()");
                    }
                    if (write(to_child_pipe[1], buff+i, 1) < 0) {
                        return_error("write()");
                    }
                }
            }
        }
        if (poll_fds[1].revents & POLLIN) {
            // from child process
            // we just echo to stdout
            int read_val = read(from_child_pipe[0], buff, BUFFER_SIZE);
            if (read_val < 0) {
                return_error("read()");
            }
            for (int i = 0; i < read_val; i++) {
                if (buff[i] == '\n') {
                    char newline_buff[2] = {'\r', '\n'};
                    if (write(STDOUT_FILENO, newline_buff, 2) < 0) {
                        return_error("write()");
                            }
                } else {
                    if (write(STDOUT_FILENO, buff+i, 1) < 0) {
                        return_error("write()");
                    }
                }
            }
        }
        if (should_exit) {
            exit(0);
        }
        if ((poll_fds[0].revents & (POLLERR | POLLHUP)) ||
            poll_fds[1].revents & (POLLERR | POLLHUP)) {
                exit(0);
        }
    }
}

int main(int argc, char *argv[]) {

    int c;
    struct option long_options[] = {
        {"shell", no_argument, NULL, SHELL},
        {0,0,0,0}
    };

    while (1) {
        int option_index = 0;
        c = getopt_long(argc, argv, "s", long_options, &option_index);

        if (c == -1) {
            break;
        }
        else if (c == SHELL) {
            shell = true;
            break;
        } else {
            print_usage();
            exit(1);
        }
    }

    // set non canonical mode
    set_non_canonical_input_mode();

    if (shell) {
        shell_mode();
    } else {
        while (1) {
            int read_val = read(0, buff, sizeof(buff));
            for (int i = 0; i < read_val; i++) {
                if (buff[i] == '\r' || buff[i] == '\n') {
                    char newline_buff[2] = {'\r', '\n'};
                    // we echo as newline but send to the shell a linefeed
                    if (write(STDOUT_FILENO, newline_buff, 2) < 0) {
                        return_error("write(1)");
                    }
                } else if (buff[i] == 4) {
                    // EOF
                    exit(0);
                } else {
                    if (write(STDOUT_FILENO, buff+i, 1) < 0) {
                        return_error("write()");
                    }
                }
            }
        }
    }
    exit(0);
}
