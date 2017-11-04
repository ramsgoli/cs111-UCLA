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

// set up boolean definitions
typedef int bool;
#define true 1
#define false 0

// command line options
#define THREADS 't'
#define ITERATIONS 'i'
#define YIELD 'y'
#define SYNC 's'

int num_threads = 1;
int num_iterations = 1;
bool opt_yield = false;

pthread_mutex_t mutex;
int lock = 0;
char sync_char = '.';

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
    if (opt_yield) {
        sched_yield();
    }
    *pointer = sum;
}

void *add_driver(void *args) {
    
    struct thread_args *struct_args = (struct thread_args*) args;
    for (int i = 0; i < num_iterations; i++) {
        switch(sync_char) {
            case '.': {
                // regular add
                add(struct_args->counter, 1);
                add(struct_args->counter, -1);
                break;
            }
            case 'm': {
                // mutex add
                pthread_mutex_lock(&mutex);
                add(struct_args->counter, 1);
                add(struct_args->counter, -1);
                pthread_mutex_unlock(&mutex);
                break;
            }
            case 's': {
                // spin lock
                while (__sync_lock_test_and_set(&lock, 1)) ;
                add(struct_args->counter, 1);
                add(struct_args->counter, -1);
                __sync_lock_release(&lock);
                break;
            }
            case 'c': {
                long long old = *struct_args->counter;
                while (old != __sync_val_compare_and_swap(struct_args->counter, old, old+1)) {
                    old = *struct_args->counter;
                    if (opt_yield) sched_yield();
                }

                old = *struct_args->counter;
                while (old != __sync_val_compare_and_swap(struct_args->counter, old, old-1)) {
                    old = *struct_args->counter;
                    if (opt_yield) sched_yield();
                }
                break;
            }
        }
        add(struct_args->counter, 1);
        add(struct_args->counter, -1);
    }

    return NULL;
}

int main(int argc, char *argv[]) {

    int c;
    struct option long_options[] = {
        {"threads", required_argument, NULL, THREADS},
        {"iterations", required_argument, NULL, ITERATIONS},
        {"yield", no_argument, NULL, YIELD},
        {"sync", required_argument, NULL, SYNC},
        {0,0,0,0}
    };

    while (true) {
        int option_index = 0;
        c = getopt_long(argc, argv, "t:i:ys:", long_options, &option_index);

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
            case YIELD: {
                opt_yield = true;
                break;
            }
            case SYNC: {
                if (optarg) {
                    switch (*optarg) {
                        case 'm': {
                            // mutex lock
                            pthread_mutex_init(&mutex, NULL);
                            sync_char = 'm';
                            break;
                        }
                        case 's': {
                            // spin lock
                            sync_char = 's';
                            break;
                        }
                        case 'c': {
                            // GCC's own atomic built in
                            sync_char = 'c';
                            break;
                        }
                        default: {
                            fprintf(stderr, "./lab2_add: invalid argument to `--sync' option\n");
                            print_usage();
                            exit(1);
                        }
                    }
                } else {
                    fprintf(stderr, "./lab2_add: --sync options requires an argument");
                    print_usage();
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

    // get the appropriate mode to print out
    char *test_name = (char*) malloc(50);
    switch (sync_char) {
        case '.': {
            if (opt_yield) {
                test_name = "add-yield-none";
            } else {
                test_name = "add-none";
            }
            break;
        }
        case 's': {
            if (opt_yield) {
                test_name = "add-yield-s";
            } else {
                test_name = "add-s";
            }
            break;
        }
        case 'm': {
            if (opt_yield) {
                test_name = "add-yield-m";
            } else {
                test_name = "add-m";
            }
            break;
        }
        case 'c': {
            if (opt_yield) {
                test_name = "add-yield-c";
            } else {
                test_name = "add-c";
            }
            break;
        }
    }

    int num_operations = num_threads * num_iterations * 2;
    int total_runtime = end_tp.tv_nsec - start_tp.tv_nsec;
    int average_runtime = total_runtime / num_operations;
    printf("%s,%d,%d,%d,%d,%d,%d\n", test_name, num_threads, num_iterations, num_operations, total_runtime, average_runtime, *args.counter);

    exit(EXIT_SUCCESS);
}
