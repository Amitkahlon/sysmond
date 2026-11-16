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
#include "ui.h"
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

int main_loop(int wait_time)
{
    while (1)
    {
        SystemState snapshot = system_state_snapshot();
        ui_draw(&snapshot, wait_time);

        int key = ui_wait_key(wait_time * 1000);
        if (key == 'q' || key == 'Q' || key == 3 /* Ctrl+C */)
        {
            return 0;
        }
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

/* Returns 0 on success, -1 if any thread failed to start. */
int start_worker_threads(int wait_time){
    int ret;

    ret = pthread_create(&threads[MEM_THREAD_ID], NULL, mem_worker_thread, (void *)(intptr_t)wait_time);
    if (ret != 0) {
        fprintf(stderr, "Failed to create mem thread: %s\n", strerror(ret));
        return -1;
    }

    ret = pthread_create(&threads[CPU_THREAD_ID], NULL, cpu_worker_thread, (void *)(intptr_t)wait_time);
    if (ret != 0) {
        fprintf(stderr, "Failed to create cpu thread: %s\n", strerror(ret));
        return -1;
    }

    ret = pthread_create(&threads[PROC_THREAD_ID], NULL, proc_worker_thread, (void *)(intptr_t)wait_time);
    if (ret != 0) {
        fprintf(stderr, "Failed to create proc thread: %s\n", strerror(ret));
        return -1;
    }

    ret = pthread_create(&threads[ALERT_THREAD_ID], NULL, alert_worker_thread, NULL);
    if (ret != 0) {
        fprintf(stderr, "Failed to create alert thread: %s\n", strerror(ret));
        return -1;
    }

    return 0;
}

void shutdown() {
    ui_restore();
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

    system_state_init();

    puts("Starting sysmond...");

    if (start_worker_threads(wait_time) != 0)
    {
        return 1;
    }

    //to ensure system state is updated
    sleep(1);

    ui_init();

    main_loop(wait_time);

    shutdown();

    return 0;
   
}