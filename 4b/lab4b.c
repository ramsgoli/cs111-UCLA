/*
NAME: Ram Goli
ID: 604751659
EMAIL: ramsgoli@gmail.com

We'll be using a thread to listen for input from STDIN, and another to listen to the temp sensor
*/

#include <signal.h>
#include <stdio.h>
#include <poll.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h> // creat()
#include <fcntl.h>
#include <pthread.h>
#include <mraa/aio.h>

// Setup our boolean definitions
typedef int bool;
#define true 1
#define false 0

// Define command line options
#define PERIOD 'p'
#define SCALE 's'
#define LOG 'l'

// program dependencies
typedef struct {
    int period;
    char scale;
    int logFD;
    time_t *rawtime;
    uint16_t sensor_value;
    float temp;
    bool should_print;
    bool can_process_input;
    mraa_aio_context temperature_sensor;
    mraa_gpio_context button;
} args_t;

void print_usage() {
/*
print the correct executable usage
*/
    fprintf(stderr, "Correct usage: ./lab4b [--period=#] [--scale=F,C] [--log=filename]\n");
}

char *get_time(args_t *args) {
/*
Returns a string formatted with the current time
*/
    tzset(); // set timezone. Useful for portability

    *(args->rawtime) = time(NULL);
    if (*(args->rawtime) == -1) {
        int err = errno;
        fprintf(stderr, "Error calling rawtime(): %s", strerror(err));
        exit(1);
    }

    struct tm *ptm = localtime(args->rawtime);
    if (ptm == NULL) {
        int err = errno;
        fprintf(stderr, "Error calling localtime(): %s", strerror(err));
        exit(1);
    }

    char *date = (char *)malloc(9 * sizeof(char)); // 9 because we have 8 chars in our formatted time + null byte
    sprintf(date, "%02d:%02d:%02d", ptm->tm_hour, 
           ptm->tm_min, ptm->tm_sec);

    return date;
}

void when_button_pressed(void *void_args) {
    args_t *args = (args_t *) void_args;

    // we need to print the final shutdown line and exit our program
    char *time = get_time(args);
    fprintf(stdout, "%s SHUTDOWN\n", time);

    if (args->logFD != -1) {
        // we also write to a log file
        dprintf(args->logFD, "%s SHUTDOWN\n", time);
    }

    exit(0);
}

void write_report(args_t *args) {
    if (!args->should_print) {
        return;
    }

    char *time = get_time(args);
    fprintf(stdout, "%s %.1f\n", time, args->temp);

    if (args->logFD != -1) {
        // we also write to a log file
        dprintf(args->logFD, "%s %.1f\n", time, args->temp);
    }

    if (!args->can_process_input) {
        args->can_process_input = true;
    }
}

void process_command(args_t *args, char *key, char *value) {
    if (strcmp(key, "SCALE") == 0) {
       args->scale = *value; 

        if (args->logFD != -1) {
            dprintf(args->logFD, "%s=%s\n", key, value);
        }
    }

    else if (strcmp(key, "PERIOD") == 0) {
        args->period = atoi(value);

        if (args->logFD != -1) {
            dprintf(args->logFD, "%s=%s\n", key, value);
        }
    }

    else if (strcmp(key, "STOP") == 0) {
        args->should_print = false;

        if (args->logFD != -1) {
            dprintf(args->logFD, "%s\n", key);
        }
    }

    else if (strcmp(key, "START") == 0) {
        args->should_print = true;
        
        if (args->logFD != -1) {
            dprintf(args->logFD, "%s\n", key);
        }
    }

    else if (strcmp(key, "OFF") == 0) {
        if (args->logFD != -1) {
            dprintf(args->logFD, "%s\n", key);
        }

        when_button_pressed((void*) args);
    }
}

void *listen_to_stdin(void *pointer_args) {
/*
Listen to STDIN
*/
    args_t *args = (args_t *) pointer_args;
    
    char buff[100];

    // the shittiest hack ever to make sure that we at least have some input
    while (!args->can_process_input) ;

    while (true) {
        int read_val = read(STDIN_FILENO, buff, 100);
        if (read_val == -1) {
            int err = errno;
            fprintf(stderr, "Error calling read(): %s", strerror(err));
        }

        char key[100];
        char value[2];

        memset(key, 0, 100);
        memset(value, 0, 2);

        int word_boundary = 0;
        for (int i = 0; i < read_val; i++) {
            if (buff[i] == '=') {
                memcpy(key, &buff[word_boundary], i - word_boundary);

                for (int j = i+1; j < read_val; j++) {
                    if (buff[j] == '\n') {
                        if (j != i+1) {
                            memcpy(value, &buff[i+1], j - i -1);                        
                        }
                        word_boundary = j + 1;
                        i = j;
                        break;
                    }
                }
                process_command(args, key, value);

                // clear our buffers
                memset(key, 0, 100);
                memset(value, 0, 2);
            } else if (buff[i] == '\n') {
                memcpy(key, &buff[word_boundary], i - word_boundary);
                word_boundary = i + 1;
                process_command(args, key, value);

                // clear our buffers
                memset(key, 0, 100);
                memset(value, 0, 2);
            }
        }
    }
    
    return NULL;
}

float convert_celsius_to_fahrenheit(float celsius) {
    return celsius * 1.8 + 32;
}

float convert_signal_to_celsius(uint16_t sensor_value) {
    const int B = 1023;  // B value of the thermister
    const int R0 = 100000;  // R0=100K

    float R = 1023.0/sensor_value-1.0;
    R = R0 * R;

    float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15; 
    return temperature;
}

void *listen_to_temperature(void *pointer_args) {
    args_t *args = (args_t *) pointer_args;

    while (true) {
        args->sensor_value = mraa_aio_read(args->temperature_sensor);

        float temp = convert_signal_to_celsius(args->sensor_value);

        if (args->scale == 'F') {
            args->temp = convert_celsius_to_fahrenheit(temp);
        } else {
            args->temp = temp;
        }
        
        write_report(args);

        //printf("period: %d\n", args->period);
        usleep(args->period * 1000000);
    }

    return NULL;
}

void *listen_to_button(void *void_args) {
    args_t *args = (args_t *) void_args;

    mraa_gpio_isr(args->button, MRAA_GPIO_EDGE_RISING, &when_button_pressed, void_args);

    return NULL;
} 

int main(int argc, char *argv[]) {

    // initialize our program dependencies
    args_t args;
    args.period = 1;
    args.scale = 'F';
    args.logFD = -1;
    args.should_print = true;
    args.can_process_input = false;

    args.temperature_sensor = mraa_aio_init(1);
    if (args.temperature_sensor == NULL) {
        fprintf(stderr, "Error initializing the temperature sensor: got NULL from init()");
        exit(1);
    }

    args.button = mraa_gpio_init(60);
    if (args.button == NULL) {
        fprintf(stderr, "Error initializing the button: got NULL from init()");
        exit(1);
    }
    mraa_gpio_dir(args.button, MRAA_GPIO_IN);

    // initialize our rawtime struct;
    args.rawtime = malloc(sizeof(time_t));

    int c;
    struct option long_options[] = {
        {"period", required_argument, NULL, PERIOD},
        {"scale", required_argument, NULL, SCALE},
        {"log", required_argument, NULL, LOG},
        {0,0,0,0}
    };

    while (true) {
        int option_index = 0;
        c = getopt_long(argc, argv, "p:s:l:", long_options, &option_index);

        if (c == -1) {
            break;
        }

        switch (c) {
            case PERIOD: {
                args.period = atoi(optarg);
                break;
            }
            case SCALE: {
                if (*optarg == 'F' || *optarg == 'C') {
                    args.scale = *optarg;
                } else {
                    print_usage();
                    exit(1);
                }
                break;
            }
            case LOG: {
                args.logFD = creat(optarg, 0666);
                if (args.logFD < 0) {
                    int err = errno;
                    fprintf(stderr, "Error calling creat(): %s", strerror(err));
                    exit(1);
                }
                break;
            }
            default: {
                print_usage();
                exit(1);
            }
        } 
    }

    // create our thread for listening to STDIN
    pthread_t stdin_thread;
    int ret = pthread_create(&stdin_thread, NULL, listen_to_stdin, (void *) &args);
    if (ret == -1) {
        int err = errno;
        fprintf(stderr, "Error calling pthread_create(): %s", strerror(err));
        exit(1);
    }

    // create thread for listening to temperature
    pthread_t temperature_thread;
    ret = pthread_create(&temperature_thread, NULL, listen_to_temperature, (void *) &args);
    if (ret == -1) {
        int err = errno;
        fprintf(stderr, "Error calling pthread_create(): %s", strerror(err));
        exit(1);
    }

    // create thread for listening to button
    pthread_t button_thread;
    ret = pthread_create(&button_thread, NULL, listen_to_button, (void *) &args);
    if (ret == -1) {
        int err = errno;
        fprintf(stderr, "Error calling pthread_create(): %s", strerror(err));
        exit(1);
    }

    ret = pthread_join(stdin_thread, NULL);
    if (ret == -1) {
        int err = errno;
        fprintf(stderr, "Error calling pthread_join(): %s", strerror(err));
        exit(1);
    }

    ret = pthread_join(temperature_thread, NULL);
    if (ret == -1) {
        int err = errno;
        fprintf(stderr, "Error calling pthread_join(): %s", strerror(err));
        exit(1);
    }
    
    ret = pthread_join(button_thread, NULL);
    if (ret == -1) {
        int err = errno;
        fprintf(stderr, "Error calling pthread_join(): %s", strerror(err));
        exit(1);
    }

    exit(0);
}