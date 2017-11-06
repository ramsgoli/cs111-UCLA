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
#include <math.h>
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
#define LISTS 'l'

typedef struct sublist_args {
    SortedList_t *head;
    int *lock;
    pthread_mutex_t *mutex;
} sublist_args_t;

// thread function arguments
typedef struct thread_args {
    int thread_num;
    long long time_waiting;
    sublist_args_t *sublist_args;
    SortedListElement_t *elements;
} thread_args_t;

int num_threads = 1;
int num_iterations = 1;
int num_lists = 1;

int opt_yield = 0;
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
        // we need to figure out with sublist this element goes into
        int sublist_index = abs(((int) *(args->elements[i].key)) % num_lists);

        switch (sync_char) {
            case '.': {
                SortedList_insert(args->sublist_args[sublist_index].head, &(args->elements[i]));
                break;
            }
            case 'm': {
                struct timespec start_tp;
                int ret = clock_gettime(CLOCK_MONOTONIC, &start_tp);
                if (ret == -1) {
                    int err = errno;
                    fprintf(stderr, "Error calling clock_gettime(): %s", strerror(err));
                    exit(1);
                }
                pthread_mutex_lock(args->sublist_args[sublist_index].mutex);

                struct timespec end_tp;
                ret = clock_gettime(CLOCK_MONOTONIC, &end_tp);
                if (ret == -1) {
                    int err = errno;
                    fprintf(stderr, "Error calling clock_gettime(): %s", strerror(err));
                    exit(1);
                }

                args->time_waiting += 1000000000L * (end_tp.tv_sec - start_tp.tv_sec) + (end_tp.tv_nsec - start_tp.tv_nsec);
                SortedList_insert(args->sublist_args[sublist_index].head, &(args->elements[i]));
                pthread_mutex_unlock(args->sublist_args[sublist_index].mutex);
                break;
            }
            case 's': {
                struct timespec start_tp;
                int ret = clock_gettime(CLOCK_MONOTONIC, &start_tp);
                if (ret == -1) {
                    int err = errno;
                    fprintf(stderr, "Error calling clock_gettime(): %s", strerror(err));
                    exit(1);
                }
                while (__sync_lock_test_and_set(args->sublist_args[sublist_index].lock, 1) == 1) ;

                struct timespec end_tp;
                ret = clock_gettime(CLOCK_MONOTONIC, &end_tp);
                if (ret == -1) {
                    int err = errno;
                    fprintf(stderr, "Error calling clock_gettime(): %s", strerror(err));
                    exit(1);
                }
                args->time_waiting += 1000000000L * (end_tp.tv_sec - start_tp.tv_sec) + (end_tp.tv_nsec - start_tp.tv_nsec);

                SortedList_insert(args->sublist_args[sublist_index].head, &(args->elements[i]));
                __sync_lock_release (args->sublist_args[sublist_index].lock);	    
                break;
            }
        }
    }

    // to obtain the length of the list, we enumerate over all the sublists
    for (int i = 0; i < num_lists; i++) {
        switch (sync_char) {
            case '.': {
                if (SortedList_length(args->sublist_args[i].head) == -1) {
                    exit(2);
                }
                break;
            }
            case 'm': {
                struct timespec start_tp;
                int ret = clock_gettime(CLOCK_MONOTONIC, &start_tp);
                if (ret == -1) {
                    int err = errno;
                    fprintf(stderr, "Error calling clock_gettime(): %s", strerror(err));
                    exit(1);
                }
                pthread_mutex_lock(args->sublist_args[i].mutex);
                struct timespec end_tp;
                ret = clock_gettime(CLOCK_MONOTONIC, &end_tp);
                if (ret == -1) {
                    int err = errno;
                    fprintf(stderr, "Error calling clock_gettime(): %s", strerror(err));
                    exit(1);
                }
                args->time_waiting += 1000000000L * (end_tp.tv_sec - start_tp.tv_sec) + (end_tp.tv_nsec - start_tp.tv_nsec);
                if (SortedList_length(args->sublist_args[i].head) == -1) {
                    exit(2);
                }
                pthread_mutex_unlock(args->sublist_args[i].mutex);
                break;
            }
            case 's': {
                struct timespec start_tp;
                int ret = clock_gettime(CLOCK_MONOTONIC, &start_tp);
                if (ret == -1) {
                    int err = errno;
                    fprintf(stderr, "Error calling clock_gettime(): %s", strerror(err));
                    exit(1);
                }
                while (__sync_lock_test_and_set(args->sublist_args[i].lock, 1) == 1) ;
                struct timespec end_tp;
                ret = clock_gettime(CLOCK_MONOTONIC, &end_tp);
                if (ret == -1) {
                    int err = errno;
                    fprintf(stderr, "Error calling clock_gettime(): %s", strerror(err));
                    exit(1);
                }
                args->time_waiting += 1000000000L * (end_tp.tv_sec - start_tp.tv_sec) + (end_tp.tv_nsec - start_tp.tv_nsec);
                if (SortedList_length(args->sublist_args[i].head) == -1) {
                    exit(2);
                }
                __sync_lock_release(args->sublist_args[i].lock);	    
                break;
            }
        }
    }
        
    for (int i = ((args->thread_num) * num_iterations); i < ((args->thread_num + 1) * num_iterations); i++) {
        int sublist_index = abs(((int) *(args->elements[i].key)) % num_lists);
        
        switch (sync_char) {
            case '.': {
                if (SortedList_lookup(args->sublist_args[sublist_index].head, args->elements[i].key) == 0) {
                    exit(2);
                }
                if (SortedList_delete(&(args->elements[i])) == 1) {
                    exit(2);
                }
                break;
            }
            case 'm': {
                pthread_mutex_lock(args->sublist_args[sublist_index].mutex);
                if (SortedList_lookup(args->sublist_args[sublist_index].head, args->elements[i].key) == 0) {
                    exit(2);
                }
                if (SortedList_delete(&(args->elements[i])) == 1) {
                    exit(2);
                }
                pthread_mutex_unlock(args->sublist_args[sublist_index].mutex);
                break;

            }
            case 's': {
                while (__sync_lock_test_and_set(args->sublist_args[sublist_index].lock, 1) == 1) ;
                if (SortedList_lookup(args->sublist_args[sublist_index].head, args->elements[i].key) == 0) {
                    exit(2);
                }
                if (SortedList_delete(&(args->elements[i])) == 1) {
                    exit(2);
                }
                __sync_lock_release (args->sublist_args[sublist_index].lock);	    
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
        {"lists", required_argument, NULL, LISTS},
        {0,0,0,0}
    };

    while (true) {
        int option_index = 0;
        c = getopt_long(argc, argv, "t:i:y:s:l:", long_options, &option_index);

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
            case LISTS: {
                num_lists = (int)atoi(optarg);
                break;
            }
            default: {
                print_usage();
                exit(1);
            }
        }
    }

    // sublist_args is an array of sublist_args_t structs that represent each sublist
    sublist_args_t *sublist_args = (sublist_args_t *) malloc(sizeof(sublist_args_t) * num_lists);

    for (int i = 0; i < num_lists; i++) {
        SortedList_t *head = (SortedList_t*) malloc(sizeof(SortedList_t));
        SortedList_t head_temp = {head, head, NULL};
        memcpy(head, &head_temp, sizeof(SortedList_t));

        pthread_mutex_t *mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
        pthread_mutex_t temp_mutex;
        pthread_mutex_init(&temp_mutex, NULL);
        memcpy(mutex, &temp_mutex, sizeof(pthread_mutex_t));

        int *lock = (int *)malloc(sizeof(int));
        int temp_lock = 0;
        memcpy(lock, &temp_lock, sizeof(int));

        sublist_args[i].head = head;
        sublist_args[i].lock = lock;
        sublist_args[i].mutex = mutex;
    }

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
    thread_args_t *t_args = (thread_args_t *) malloc(sizeof(thread_args_t) * num_threads);
    pthread_t *threads = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);

    // create all threads
    for (int i = 0; i < num_threads; i++) {
        t_args[i].sublist_args = sublist_args;
        t_args[i].elements = elements;
        t_args[i].thread_num = i;
        t_args[i].time_waiting = 0;
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
    
    for (int i = 0; i < num_lists; i++) {
        if (SortedList_length(sublist_args[i].head) != 0) {
            exit(2);
        }
    }

    // get total lock acquisition time
    long long total_acquisition_time = 0;
    for (int i = 0; i < num_threads; i++) {
        total_acquisition_time += t_args[i].time_waiting;
    }

    // get the appropriate mode to print out
    char *yield_opts = yield_options();
    char *sync_opts = sync_options(); 

    int num_operations = num_threads * num_iterations * 3;
    long long wait_for_lock = total_acquisition_time / num_operations;
    long total_runtime = 1000000000L * (end_tp.tv_sec - start_tp.tv_sec) + (end_tp.tv_nsec - start_tp.tv_nsec);
    int average_runtime = total_runtime / num_operations;
    printf("list-%s-%s,%d,%d,%d,%d,%ld,%d,%lld\n", yield_opts, sync_opts, num_threads, num_iterations, num_lists, num_operations, total_runtime, average_runtime, wait_for_lock);

    // FREE ALL THE THINGS
    free(threads);
    free(t_args);

    for (int i = 0; i < num_list_elements; i++) {
        free((char*) elements[i].key);
    }
    free(elements);

    for (int i = 0; i < num_lists; i++) {
        free(sublist_args[i].head);
        free(sublist_args[i].lock);
        free(sublist_args[i].mutex);
    }
    free(sublist_args);
    
    exit(EXIT_SUCCESS);
}