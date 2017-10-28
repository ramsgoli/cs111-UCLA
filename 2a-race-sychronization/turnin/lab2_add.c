/*
NAME: Ram Goli
ID: 604751659
EMAIL: ramsgoli@gmail.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <string.h>

// command line options
#define THREADS 't'
#define ITERATIONS 'i'

int num_threads = 1;
int num_iterations = 1;

// set up boolean definitions
typedef int bool;
#define true 1
#define false 0

// thread function arguments
struct thread_args {
    long long *counter;
};

void print_usage() {
/*
prints an error message in case bad arguments are passed on the command line
*/
    fprintf(stderr, "%s\n", "Correct usage: ./lab2_add [--threads=#][--iteration=#]");
}

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    *pointer = sum;
}

void *add_driver(void *args) {
    
    struct thread_args *struct_args = (struct thread_args*) args;
    for (int i = 0; i < num_iterations; i++) {
        add(struct_args->counter, 1);
        add(struct_args->counter, -1);
    }
}

int main(int argc, char *argv[]) {

    int c;
    struct option long_options[] = {
        {"threads", required_argument, NULL, THREADS},
        {"iterations", required_argument, NULL, ITERATIONS},
        {0,0,0,0}
    };

    while (true) {
        int option_index = 0;
        c = getopt_long(argc, argv, "t:i:", long_options, &option_index);

        if (c == -1) {
            break;
        }

        switch (c) {
            case THREADS: {
                num_threads = (int)atoi(optarg);
                break;
            }
            case ITERATIONS: {
                num_iterations = (int)atoi(optarg);
                break;
            }
            default: {
                print_usage();
                exit(1);
            }
        }
    }

    // get the start time
    struct timespec start_tp;
    int ret = clock_gettime(CLOCK_MONOTONIC, &start_tp);
    if (ret == -1) {
        int err = errno;
        fprintf(stderr, "Error calling clock_gettime(): %s", strerror(err));
        exit(1);
    }

    // initialize our threads
    pthread_t threads[num_threads];
    long long counter = 0;

    struct thread_args args;
    args.counter = &counter;

    // create all threads
    for (int i = 0; i < num_threads; i++) {
        ret = pthread_create(&threads[i], NULL, add_driver, (void *) &args);
        if (ret == -1) {
            int err = errno;
            fprintf(stderr, "Error calling pthread_create(): %s", strerror(err));
            exit(1);
        }
    }

    // wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        ret = pthread_join(threads[i], NULL);
        if (ret == -1) {
            int err = errno;
            fprintf(stderr, "Error calling pthread_create(): %s", strerror(err));
            exit(1);
        }
    }

    // get the end time
    struct timespec end_tp;
    ret = clock_gettime(CLOCK_MONOTONIC, &end_tp);
    if (ret == -1) {
        int err = errno;
        fprintf(stderr, "Error calling clock_gettime(): %s", strerror(err));
        exit(1);
    }

    int num_operations = num_threads * num_iterations * 2;
    int total_runtime = end_tp.tv_nsec - start_tp.tv_nsec;
    int average_runtime = total_runtime / num_operations;
    printf("%s,%d,%d,%d,%d,%d,%d\n", "add-none", num_threads, num_iterations, num_operations, total_runtime, average_runtime, *args.counter);
}