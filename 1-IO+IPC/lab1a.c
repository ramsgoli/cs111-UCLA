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

struct termios old_term_attrs;
const int BUFFER_SIZE = 128;
char *buff;

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

void map_to_newline(int size) {
    char *new_buff = (char *)malloc(BUFFER_SIZE);
    int i = 0;

    while (i < size) {
        if (buff[i] == '\r' || buff[i] == '\n') {
            new_buff[i] = '\r';
            i++;
            new_buff[i] = '\n';
        }
        else {
            new_buff[i] = buff[i];
        }
        i++;
    }
    free(buff);
    buff = new_buff;
}

void read_input() {
    buff = (char*)malloc(BUFFER_SIZE);
    int read_val;

    while (1) {
        int num_delimiters = 0;
        read_val = read(0, buff, sizeof(buff));
        for (int i = 0; i < read_val; i++) {
            if (buff[i] == '\r' || buff[i] == '\n') {
                num_delimiters++;
            }
            if (buff[i] == 4) {
                // EOF
                return;
            }
            if (buff[i] == 3) {
                // SIGINT. Kill the process
                kill(getpid(), SIGINT);
            }

        }
        // map any delimeter characters (like '\r' or '\n' to '\r\n'), if any
        if (num_delimiters) {
            map_to_newline(read_val + num_delimiters);
        }

        int write_amount = write(1, buff, read_val + num_delimiters);
        if (write_amount == -1) {
            return_error("write()");
        }
    }
    free(buff);
}

int main() {
    set_non_canonical_input_mode();
    read_input();
    exit(EXIT_SUCCESS);
}
