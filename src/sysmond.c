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

bool cpu_sensor_status = true;
bool proc_sensor_status = true;
bool mem_sensor_status = true;

int pid_mem_sensor;
int pid_cpu_sensor;
int pid_proc_sensor;

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

int main_loop(int wait_time)
{
    while (1)
    {
        sleep(wait_time);

        time_t now = time(NULL);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

        printf("\n========== System Status [%s] ==========\n", timestamp);
        printf("--- Memory ---\n");
        if (mem_sensor_status)
        {
            read_mem_file();
        }
        else
        {
            printf("** mem sensor is down **\n");
        }
        printf("--- CPU Load ---\n");
        if (cpu_sensor_status)
        {
            read_cpu_file();
        }
        else
        {
            printf("** cpu sensor is down **\n");
        }
        printf("--- Processes ---\n");
        if (proc_sensor_status)
        {
            read_proc_file();
        }
        else
        {
            printf("** proc sensor is down **\n");
        }

        printf("============END============\n");
        fflush(stdout);
    }
}

void handle_sigchld(int sig)
{
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (pid == pid_cpu_sensor)
        {
            cpu_sensor_status = false;
        }
        else if (pid == pid_proc_sensor)
        {
            proc_sensor_status = false;
        }
        else if (pid == pid_mem_sensor)
        {
            mem_sensor_status = false;
        }
    }
}

int main(int argc, char *argv[])
{

    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction failed");
        return 1;
    }

    mkdir(OUTPUT_FOLDER, 0755);
    int wait_time = DEFAULT_WAIT_TIME;

    pid_mem_sensor = fork();
    if (pid_mem_sensor == 0)
    {
        // child mem_sensor

        char *memargv[] = {"bin/mem_sensor", NULL};
        execv("bin/mem_sensor", memargv);

        perror("execv failed, mem_sensor");
        exit(1);
    }
    else
    {
        // base
        pid_cpu_sensor = fork();

        if (pid_cpu_sensor == 0)
        {
            // child cpu_sensor
            char *cpuargv[] = {"bin/cpu_sensor", NULL};
            execv("bin/cpu_sensor", cpuargv);

            perror("execv failed, cpu_sensor");
            exit(1);
        }
        else
        {
            // base
            pid_proc_sensor = fork();
            if (pid_proc_sensor == 0)
            {
                // child proc_sensor
                char *cpuargv[] = {"bin/proc_sensor", NULL};
                execv("bin/proc_sensor", cpuargv);

                perror("execv failed, proc_sensor");
                exit(1);
            }
            else
            {
                // execute loop
                main_loop(wait_time);
            }
        }
    }
}