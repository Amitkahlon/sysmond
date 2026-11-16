#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include "config.h"
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "mem_sensor.h"
#include "cpu_sensor.h"
#include "proc_sensor.h"
#include "system_state.h"
#include "mem_sensor.h"
#include "cpu_sensor.h"
#include "proc_sensor.h"
#include "alert_thread.h"
#include <pthread.h>
#include <stdint.h>

enum {
    MEM_THREAD_ID = 0,
    CPU_THREAD_ID,
    PROC_THREAD_ID,
    ALERT_THREAD_ID,
    THREAD_COUNT /* always last: equals the number of threads (4) */
};



static pthread_t threads[THREAD_COUNT];

/* depreceted*/
static bool cpu_sensor_status = true;
static bool proc_sensor_status = true;
static bool mem_sensor_status = true;


static void print_mem_data(const SystemState *sys_state)
{
    if (!mem_sensor_status)
    {
        // print error
        return;
    }

    printf("Mem Total: %lu kB\n", sys_state->mem_total);
    printf("Mem Free: %lu kB\n", sys_state->mem_free);
    printf("Mem Available: %lu kB\n", sys_state->mem_avail);
    printf("Swap Total: %lu kB\n", sys_state->swap_total);
    printf("Swap Free: %lu kB\n", sys_state->swap_free);
}

void print_cpu_data(const SystemState *sys_state)
{
    if (!cpu_sensor_status)
    {
        // print error
        return;
    }
    printf("1-minute load average: %.2f\n", sys_state->load1);
    printf("5-minute load average: %.2f\n", sys_state->load5);
    printf("15-minute load average: %.2f\n", sys_state->load15);
}

void print_proc_data(const SystemState *sys_state)
{
    if (!proc_sensor_status)
    {
        // print error
        return;
    }
    printf("Active processes count: %d\n", sys_state->total_procs);
}

void print_time_now(){
        time_t now = time(NULL);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
        printf("%s\n", timestamp);
}

int main_loop(int wait_time)
{
    SystemState snapshot = system_state_snapshot(); 
    while (1)
    {
        sleep(wait_time);

        printf("\n========== System Status ==========\n");
        print_time_now();
        printf("--- Memory ---\n");
        print_mem_data(&snapshot);
        printf("--- CPU Load ---\n");
        print_cpu_data(&snapshot);
        printf("--- Processes ---\n");
        print_proc_data(&snapshot);
        printf("============END============\n");
        fflush(stdout);
    }
}


void ensure_output_dir_exists() {
    mkdir(OUTPUT_FOLDER, 0755);
}


void wait_all_threads() {
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
}

void start_worker_threads(int wait_time){

    if (pthread_create(&threads[MEM_THREAD_ID], NULL, mem_worker_thread, (void *)(intptr_t)wait_time) != 0) {
        perror("Failed to create thread 1");
        return 1;
    }

    if (pthread_create(&threads[CPU_THREAD_ID], NULL, cpu_worker_thread, (void *)(intptr_t)wait_time) != 0) {
        perror("Failed to create thread 2");
        return 1;
    }

    if (pthread_create(&threads[PROC_THREAD_ID], NULL, proc_worker_thread, (void *)(intptr_t)wait_time) != 0) {
        perror("Failed to create thread 3");
        return 1;
    }

    if (pthread_create(&threads[ALERT_THREAD_ID], NULL, alert_worker_thread, NULL) != 0) {
        perror("Failed to create thread 3");
        return 1;
    }
}

void shutdown() {
    wait_all_threads();
}


int main(int argc, char *argv[])
{
    int wait_time = DEFAULT_WAIT_TIME;

    if (argc > 1)
    {
        wait_time = atoi(argv[1]);
    }

    ensure_output_dir_exists();

    start_worker_threads(wait_time);

    main_loop(wait_time);

    return 0;
   
}