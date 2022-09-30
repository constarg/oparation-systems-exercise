#ifdef __unix__

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#endif

#ifndef __unix__
#include <stdio.h>
#endif

#define TRUE  1
#define FALSE 0


#define FAILED_TO_CREATE_THREAD(THREAD)                         \
{                                                               \
        printf("Failed to create %s thread..\n", THREAD);       \
        exit(1);                                                \
}

#define ERROR() exit(EXIT_FAILURE);

struct thread_run_info {
    int time_to_run;
    int what_to_exec;
    int is_exec_random: 1;
    char name[8];
};

static int number = 0;


static inline void change_number(int x)
{
    number += x;
}

static void print_thread_attr(pthread_t thread,
                              const char *name, int wait_time)
{
    printf("---------- %s thread attributes ----------\n", name);
    printf("The thread run for: %d secs\n\n", wait_time);
    pthread_attr_t thread_attr_t;
    int error_code = 0;
    int status = 0;
    size_t guard_size = 0;
    size_t stack_size = 0;
    void *stack_addr = NULL;
    struct sched_param sched_param_t;

    error_code = pthread_getattr_np(thread, &thread_attr_t);
    if (error_code != 0) ERROR()

    // Παίρνουμε την τιμη του detache state.
    error_code = pthread_attr_getdetachstate(&thread_attr_t, &status);
    if (error_code != 0) ERROR()

    printf("Detach state        = %s\n",
                   (status == PTHREAD_CREATE_DETACHED) ? "PTHREAD_CREATE_DETACHED" :
                   (status == PTHREAD_CREATE_JOINABLE) ? "PTHREAD_CREATE_JOINABLE" :
                   "???");

    error_code = pthread_attr_getscope(&thread_attr_t, &status);
    if (error_code != 0) ERROR()

    printf("Scope               = %s\n",
                   (status == PTHREAD_SCOPE_SYSTEM)  ? "PTHREAD_SCOPE_SYSTEM" :
                   (status == PTHREAD_SCOPE_PROCESS) ? "PTHREAD_SCOPE_PROCESS" :
                   "???");

    error_code = pthread_attr_getinheritsched(&thread_attr_t, &status);
    if (error_code != 0) ERROR()

    printf("Inherit scheduler   = %s\n",
                   (status == PTHREAD_INHERIT_SCHED)  ? "PTHREAD_INHERIT_SCHED" :
                   (status == PTHREAD_EXPLICIT_SCHED) ? "PTHREAD_EXPLICIT_SCHED" :
                   "???");

    error_code = pthread_attr_getschedpolicy(&thread_attr_t, &status);
    if (error_code != 0) ERROR()

    printf("Scheduling policy   = %s\n",
                   (status == SCHED_OTHER) ? "SCHED_OTHER" :
                   (status == SCHED_FIFO)  ? "SCHED_FIFO" :
                   (status == SCHED_RR)    ? "SCHED_RR" :
                   "???");

    error_code = pthread_attr_getschedparam(&thread_attr_t, &sched_param_t);
    if (error_code != 0) ERROR()

    printf("Scheduling priority = %d\n", sched_param_t.sched_priority);

    error_code = pthread_attr_getguardsize(&thread_attr_t, &guard_size);
    if (error_code != 0) ERROR()

    printf("Guard size          = %zu bytes\n", guard_size);

    error_code = pthread_attr_getstack(&thread_attr_t, &stack_addr, &stack_size);
    if (error_code != 0) ERROR()

    printf("Stack address       = %p\n", stack_addr);
    printf("Stack size          = %#zx bytes\n\n", stack_size);

    pthread_attr_destroy(&thread_attr_t);
    printf("Number value after thread call: %d\n", number);
}

static void *thread_run(void *thread_info)
{
    clock_t thread_start_time = clock();
    clock_t time_spend;
    struct thread_run_info *thread_run_info = (struct thread_run_info *)thread_info;
    int curr_run_time = 0;
    srand(time(NULL));

    if (thread_start_time == -1) pthread_exit(0); // error.

    do
    {
        // Βρίσκουμε τον χρόνο που έχει περάσει ( σε κύκλους ρολογιού ).
        time_spend = clock() - thread_start_time;
        if (time_spend < 0) pthread_exit(0); // error.

        if (thread_run_info->is_exec_random)
            change_number(-(1 + rand() % (10 + 1 - 1)));
        else
            change_number(thread_run_info->what_to_exec);

        curr_run_time = time_spend / CLOCKS_PER_SEC;
    } while (curr_run_time < thread_run_info->time_to_run);

    print_thread_attr(pthread_self(), thread_run_info->name, thread_run_info->time_to_run);
}

static void child_process_exec(void)
{
    pthread_t minus_t;
    pthread_t increase_t;
    pthread_attr_t minus_attr_t;
    pthread_attr_t increase_attr_t;

    if (pthread_attr_init(&minus_attr_t) != 0) ERROR()
    if (pthread_attr_init(&increase_attr_t) != 0) ERROR()

    srand(time(NULL));
    int minus_t_rand = 1 + rand() % (10 + 1 - 1);
    int increase_t_rand = 1 + rand() % (10 + 1 - 1);

    struct thread_run_info minus_t_run_info;
    memcpy(&minus_t_run_info.time_to_run, &minus_t_rand, sizeof(int));
    minus_t_run_info.what_to_exec = 0;
    minus_t_run_info.is_exec_random = TRUE;
    strcpy(minus_t_run_info.name, "minus");

    struct thread_run_info increase_t_run_info;
    memcpy(&increase_t_run_info.time_to_run, &increase_t_rand, sizeof(int));
    increase_t_run_info.what_to_exec = 5;
    increase_t_run_info.is_exec_random = FALSE;
    strcpy(increase_t_run_info.name, "increase");

    if (pthread_create(&minus_t, &minus_attr_t, &thread_run,
                       (void *)&minus_t_run_info) != 0) FAILED_TO_CREATE_THREAD("minus")
    if (pthread_create(&increase_t, &increase_attr_t, &thread_run,
                       (void *)&increase_t_run_info) != 0) FAILED_TO_CREATE_THREAD("increase")

    //pthread_join(minus_t, NULL);
    pthread_join(increase_t, NULL);
}

int main(int argc, char **argv)
{

#ifdef __unix__
    pid_t proc_id = fork();

    if (proc_id == 0)
        child_process_exec();
    else
        wait(0);
#else
    printf("The oparation system is not based on POSIX", );
#endif

    return 0;
}
