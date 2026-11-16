#include "system_state.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>

static SystemState s_state;

void system_mem_set(long mem_total, long mem_free, long mem_avail, long swap_total, long swap_free)
{
    pthread_mutex_lock(&s_state.lock);

    s_state.mem_total = mem_total;
    s_state.mem_free = mem_free;
    s_state.mem_avail = mem_avail;
    s_state.swap_total = swap_total;
    s_state.swap_free = swap_free;

    pthread_mutex_unlock(&s_state.lock);
}

void system_cpu_set(double load1, double load5, double load15)
{
    pthread_mutex_lock(&s_state.lock);

    s_state.load1 = load1;
    s_state.load5 = load5;
    s_state.load15 = load15;

    pthread_cond_broadcast(&s_state.alert_cond);

    pthread_mutex_unlock(&s_state.lock);
}
void system_proc_set(int total_procs)
{
    pthread_mutex_lock(&s_state.lock);

    s_state.total_procs = total_procs;

    pthread_mutex_unlock(&s_state.lock);
}

SystemState system_state_snapshot()
{
    SystemState state_copy;

    pthread_mutex_lock(&s_state.lock);

    state_copy.mem_total = s_state.mem_total;
    state_copy.mem_free = s_state.mem_free;
    state_copy.mem_avail = s_state.mem_avail;
    state_copy.swap_total = s_state.swap_total;
    state_copy.swap_free = s_state.swap_free;
    state_copy.load1 = s_state.load1;
    state_copy.load5 = s_state.load5;
    state_copy.load15 = s_state.load15;
    state_copy.total_procs = s_state.total_procs;
    
    pthread_mutex_unlock(&s_state.lock);

    return state_copy;
}

void system_state_init()
{
    pthread_mutex_init(&s_state.lock, NULL);
    pthread_cond_init(&s_state.alert_cond, NULL);
}

/* Blocks until the 1-minute load goes above threshold, then prints one alert. */
void system_wait_for_load_above(double threshold)
{
    pthread_mutex_lock(&s_state.lock);

    while (s_state.load1 <= threshold) {
        pthread_cond_wait(&s_state.alert_cond, &s_state.lock);
    }

    fprintf(stderr, "Monitor: Alert! Value is now %f (exceeded %f)!\n", s_state.load1, threshold);

    pthread_mutex_unlock(&s_state.lock);
}

/* Blocks until the 1-minute load drops back to threshold or below. */
void system_wait_for_load_below(double threshold)
{
    pthread_mutex_lock(&s_state.lock);

    while (s_state.load1 > threshold) {
        pthread_cond_wait(&s_state.alert_cond, &s_state.lock);
    }

    pthread_mutex_unlock(&s_state.lock);
}