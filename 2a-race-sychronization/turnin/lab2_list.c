/*
NAME: Ram Goli
ID: 604751659
EMAIL: ramsgoli@gmail.com
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include "SortedList.h"

// set up boolean definitions
typedef int bool;
#define true 1
#define false 0

// command line options
#define THREADS 't'
#define ITERATIONS 'i'
#define YIELD 'y'
#define SYNC 's'

// thread function arguments
typedef struct thread_args {
    int thread_num;
    SortedList_t *head;
    SortedListElement_t *elements;
} thread_args_t;

int num_threads = 1;
int num_iterations = 1;

pthread_mutex_t mutex;
int opt_yield = 0;
int lock = 0;
char sync_char = '.';

void print_usage() {
/*
prints an error message in case bad arguments are passed on the command line
*/
    fprintf(stderr, "%s\n", "Correct usage: ./lab2_list [--threads=#][--iteration=#]");
}

void *add_to_list(void *pointer) {
    thread_args_t *args = (thread_args_t *) pointer;

    // we access num_iterations elements starting at the index of the thread_num
    for (int i = (args->thread_num * num_iterations); i < ((args->thread_num + 1) * num_iterations); i++) {
        switch (sync_char) {
            case '.': {
                SortedList_insert(args->head, &(args->elements[i]));
                break;
            }
            case 'm': {
                pthread_mutex_lock(&mutex);
                SortedList_insert(args->head, &(args->elements[i]));
                pthread_mutex_unlock(&mutex);
                break;
            }
            case 's': {
                while (__sync_lock_test_and_set(&lock, 1) == 1) ;
                SortedList_insert(args->head, &(args->elements[i]));
                __sync_lock_release (&lock);	    
                break;
            }
        }
    }

    switch (sync_char) {
        case '.': {
            if (SortedList_length(args->head) == -1) {
                exit(2);
            }
            break;
        }
        case 'm': {
            pthread_mutex_lock(&mutex);
            if (SortedList_length(args->head) == -1) {
                exit(2);
            }
            pthread_mutex_unlock(&mutex);
            break;
        }
        case 's': {
            while (__sync_lock_test_and_set(&lock, 1) == 1) ;
            if (SortedList_length(args->head) == -1) {
                exit(2);
            }
            __sync_lock_release(&lock);	    
            break;
        }
    }
        
    for (int i = ((args->thread_num) * num_iterations); i < ((args->thread_num + 1) * num_iterations); i++) {
        switch (sync_char) {
            case '.': {
                if (SortedList_lookup(args->head, args->elements[i].key) == 0) {
                    exit(2);
                }
                if (SortedList_delete(&(args->elements[i])) == 1) {
                    exit(2);
                }
                break;
            }
            case 'm': {
                pthread_mutex_lock(&mutex);
                if (SortedList_lookup(args->head, args->elements[i].key) == 0) {
                    exit(2);
                }
                if (SortedList_delete(&(args->elements[i])) == 1) {
                    exit(2);
                }
                pthread_mutex_unlock(&mutex);
                break;

            }
            case 's': {
                while (__sync_lock_test_and_set(&lock, 1) == 1) ;
                if (SortedList_lookup(args->head, args->elements[i].key) == 0) {
                    exit(2);
                }
                if (SortedList_delete(&(args->elements[i])) == 1) {
                    exit(2);
                }
                __sync_lock_release (&lock);	    
                break;
            }
        }
    }
    return NULL;
}

char* sync_options() {
    char* sync = (char*)malloc(5);
    memset(sync, 0, 5);

    switch (sync_char) {
        case '.': {
            sync[0] = 'n';
            sync[1] = 'o';
            sync[2] = 'n';
            sync[3] = 'e';
            break;
        }
        case 's': {
            sync[0] = 's';
            break;
        }
        case 'm': {
            sync[0] = 'm'; 
            break;
        }
    }
    return sync;
}

char* yield_options() {
    char* yield = (char*) malloc(5*sizeof(char));
    memset(yield, 0, 5);

    int index = 0;
    if (opt_yield & INSERT_YIELD) {
        yield[index] = 'i';
        index++;
    }
    if (opt_yield & DELETE_YIELD) {
        yield[index] = 'd';
        index++;
    }
    if (opt_yield & LOOKUP_YIELD) {
        yield[index] = 'l';
        index++;
    }

    if (index == 0) {
        yield[0] = 'n';
        yield[1] = 'o';
        yield[2] = 'n';
        yield[3] = 'e';
    }
    return yield;
}

int main(int argc, char *argv[]) {
    // initialize our thread args and give it the default values

    int c;
    struct option long_options[] = {
        {"threads", required_argument, NULL, THREADS},
        {"iterations", required_argument, NULL, ITERATIONS},
        {"yield", required_argument, NULL, YIELD},
        {"sync", required_argument, NULL, SYNC},
        {0,0,0,0}
    };

    while (true) {
        int option_index = 0;
        c = getopt_long(argc, argv, "t:i:y:s:", long_options, &option_index);

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
                opt_yield = 0x00;
                int len = strlen(optarg);
                for (int i = 0; i < len; i++) {
                    switch (optarg[i]) {
                        case 'i': {
                            opt_yield = opt_yield | INSERT_YIELD;
                            break;
                        }
                        case 'd': {
                            opt_yield = opt_yield | DELETE_YIELD;
                            break;
                        }
                        case 'l': {
                            opt_yield = opt_yield | LOOKUP_YIELD;
                            break;
                        }
                        default: {
                            print_usage();
                            exit(1);
                        }
                    }
                }
                break;
            }
            case SYNC: {
                if (optarg) {
                    switch (*optarg) {
                        case 'm': {
                            pthread_mutex_init(&mutex, NULL);
                            sync_char = 'm';
                            break;
                        }
                        case 's': {
                            sync_char = 's';
                            break;
                        }
                        case 'c': {
                            sync_char = 'c';
                            break;
                        }
                        default: {
                            print_usage();
                            exit(1);
                        }
                    }
                } else {
                    // need an argument
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

    // initialize an empty list
    SortedList_t *head = (SortedList_t*) malloc(sizeof(SortedList_t));
    SortedList_t temp = {head, head, NULL};
    memcpy(head, &temp, sizeof(SortedList_t));

    int num_list_elements = num_iterations * num_threads;
    
    SortedListElement_t *elements = (SortedListElement_t*) malloc(sizeof(SortedListElement_t) * num_list_elements);
    srand(time(NULL));
    for (int i = 0; i < num_list_elements; i++) {
        char *temp_key = (char*)malloc(sizeof(char));
        *temp_key = (char)(rand() % 256);
        SortedListElement_t temp2 = {NULL, NULL, temp_key};
        memcpy(&elements[i], &temp2, sizeof(SortedListElement_t));
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
    thread_args_t t_args[num_threads];
    pthread_t threads[num_threads];

    // create all threads
    for (int i = 0; i < num_threads; i++) {
        t_args[i].head = head;
        t_args[i].elements = elements;
        t_args[i].thread_num = i;
        ret = pthread_create(&threads[i], NULL, add_to_list, (void *) &t_args[i]);
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
    
    if (SortedList_length(head) != 0) {
        exit(2);
    }

    // get the appropriate mode to print out
    char *yield_opts = yield_options();
    char *sync_opts = sync_options(); 

    int num_operations = num_threads * num_iterations * 3;
    long total_runtime = 1000000000L * (end_tp.tv_sec - start_tp.tv_sec) + (end_tp.tv_nsec - start_tp.tv_nsec);
    int average_runtime = total_runtime / num_operations;
    printf("list-%s-%s,%d,%d,1,%d,%ld,%d\n", yield_opts, sync_opts, num_threads, num_iterations, num_operations, total_runtime, average_runtime);
}