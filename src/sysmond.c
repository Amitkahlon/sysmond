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


struct Mem_Data
{
    unsigned long mem_total;
    unsigned long mem_free;
    unsigned long mem_avaliable;
    unsigned long swap_total;
    unsigned long swap_free;
};

struct Cpu_Data
{
    float min1_avg;
    float min5_avg;
    float min15_avg;
};

void print_mem_data(const struct Mem_Data *mem_data)
{
    printf("MemTotal: %lu kB\n", mem_data->mem_total);
    printf("MemFree: %lu kB\n", mem_data->mem_free);
    printf("MemAvailable: %lu kB\n", mem_data->mem_avaliable);
    printf("SwapTotal: %lu kB\n", mem_data->swap_total);
    printf("SwapFree: %lu kB\n", mem_data->swap_free);
}

void print_cpu_data(const struct Cpu_Data cpu_data)
{
    printf("1-minute load average:   %.2f\n", cpu_data.min1_avg);
    printf("5-minute load average:   %.2f\n", cpu_data.min5_avg);
    printf("15-minute load average:  %.2f\n", cpu_data.min15_avg);
}

void read_mem_file()
{
    struct Mem_Data mem_data = {0};
    FILE *fd = fopen(MEM_OUTPUT_PATH, "r");

    if (fd == NULL)
    {
        perror(MEM_OUTPUT_PATH);
        return;
    }

    if (fscanf(fd, "=== Memory Information === Actual time elapsed: %*f seconds "
                   "Mem Total: %lu kB Mem Free: %lu kB Mem Available: %lu kB "
                   "Swap Total: %lu kB Swap Free: %lu kB",
               &mem_data.mem_total, &mem_data.mem_free, &mem_data.mem_avaliable,
               &mem_data.swap_total, &mem_data.swap_free) == 5)
    {
        print_mem_data(&mem_data);
    }

    fclose(fd);
}

void read_cpu_file()
{
    struct Cpu_Data cpu_data = {0};
    FILE *fd = fopen(CPU_OUTPUT_PATH, "r");

    if (fd == NULL)
    {
        perror(CPU_OUTPUT_PATH);
        return;
    }

    if (fscanf(fd, "1 Minute Averge: %f 5 Minute Averge: %f 15 Minute Averge: %f",
               &cpu_data.min1_avg, &cpu_data.min5_avg, &cpu_data.min15_avg) == 3)
    {
        print_cpu_data(cpu_data);
    }

    fclose(fd);
}

void read_proc_file()
{
    int proc_count = 0;
    FILE *fd = fopen(PROC_OUTPUT_PATH, "r");

    if (fd == NULL)
    {
        perror(PROC_OUTPUT_PATH);
        return;
    }

    if (fscanf(fd, "active proc count: %d", &proc_count) == 1)
    {
        printf("proc count: %d\n", proc_count);
    }

    fclose(fd);
}

int main(int argc, char *argv[]) {
    mkdir(OUTPUT_FOLDER, 0755);
    int wait_time = DEFAULT_WAIT_TIME;

    while (1) {

        sleep(wait_time);

        time_t now = time(NULL);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

        printf("\n========== System Status [%s] ==========\n", timestamp);
        printf("--- Memory ---\n");
        read_mem_file();
        printf("--- CPU Load ---\n");
        read_cpu_file();
        printf("--- Processes ---\n");
        read_proc_file();
        fflush(stdout);
    }

}