// my_struct.h
#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <pthread.h>

typedef struct {
    // Memory Metrics
    long mem_total;
    long mem_free;
    long mem_avail;
    long swap_total;
    long swap_free;

    // CPU Load Metrics
    double load1;
    double load5;
    double load15;

    // Process Metrics
    int total_procs;

    // Synchronization
    pthread_mutex_t lock;
    pthread_cond_t alert_cond;
} SystemState;


void system_state_init();
void system_mem_set(long mem_total, long mem_free, long mem_avail, long swap_total, long swap_free);
void system_cpu_set(double load1, double load5, double load15);
void system_proc_set(int total_procs);
SystemState system_state_snapshot();
void system_wait_for_load_above(double threshold);
void system_wait_for_load_below(double threshold);


#endif // SYSTEM_STATE_H