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
#include "sysmon_ipc.h"
#include <fcntl.h>

#define MEM_BIN_PATH "bin/mem_sensor"
#define CPU_BIN_PATH "bin/cpu_sensor"
#define PROC_BIN_PATH "bin/proc_sensor"

static int pid_mem_sensor = -1;
static int pid_cpu_sensor = -1;
static int pid_proc_sensor = -1;

int mem_pipe_fd[2] = {-1, -1};
static int cpu_fifo_fd = -1;
static int proc_fifo_fd = -1;

static void shutdown_handler(int sig)
{
    int pids[] = {pid_mem_sensor, pid_cpu_sensor, pid_proc_sensor};

    for (int i = 0; i < 3; i++)
        if (pids[i] > 0)
            kill(pids[i], SIGTERM);

    for (int i = 0; i < 3; i++)
        if (pids[i] > 0)
            waitpid(pids[i], NULL, 0);

     close(mem_pipe_fd[0]);
     close(mem_pipe_fd[1]);
    close(cpu_fifo_fd);
    close(proc_fifo_fd);

    unlink(CPU_FIFO_PATH);
    unlink(PROC_FIFO_PATH);

    _exit(0);
}

static void install_signal_handlers(void)
{
    struct sigaction sa = {0};
    sa.sa_handler = shutdown_handler;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGTERM);

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

void print_cpu_data(const CpuData *data)
{
    if (data == NULL)
    {
        fprintf(stderr, "CpuData is NULL\n");
        return;
    }

    printf("Load 1m : %.2f\n", data->load1);
    printf("Load 5m : %.2f\n", data->load5);
    printf("Load 15m: %.2f\n", data->load15);
}

void print_mem_data(const MemData *data)
{
    if (data == NULL)
    {
        fprintf(stderr, "MemData is NULL\n");
        return;
    }

    printf("Mem Total : %ld\n", data->mem_total);
    printf("Mem Free  : %ld\n", data->mem_free);
    printf("Mem Avail : %ld\n", data->mem_avail);
    printf("Swap Total: %ld\n", data->swap_total);
    printf("Swap Free : %ld\n", data->swap_free);
}

void print_proc_data(const ProcData *data)
{
    if (data == NULL)
    {
        fprintf(stderr, "ProcData is NULL\n");
        return;
    }

    printf("Total Procs: %d\n", data->total_procs);
}

CpuData read_cpu()
{
    CpuData buff = {0};

    cpu_fifo_fd = open(CPU_FIFO_PATH, O_RDONLY);
    if (cpu_fifo_fd == -1)
    {
        perror("parent read open failed");
        return buff;
    }

    ssize_t n = read(cpu_fifo_fd, &buff, sizeof(buff));
    if (n != sizeof(buff))
        fprintf(stderr, "read_cpu: short read (%zd)\n", n);

    close(cpu_fifo_fd);
    cpu_fifo_fd = -1;

    return buff;
}

ProcData read_proc()
{
    ProcData buff = {0};

    proc_fifo_fd = open(PROC_FIFO_PATH, O_RDONLY);
    if (proc_fifo_fd == -1)
    {
        perror("parent read open failed");
        return buff;
    }

    ssize_t n = read(proc_fifo_fd, &buff, sizeof(buff));
    if (n != sizeof(buff))
        fprintf(stderr, "read_proc: short read (%zd)\n", n);

    close(proc_fifo_fd);
    proc_fifo_fd = -1;

    return buff;
}

MemData read_mem()
{
    MemData buf = {0};
    ssize_t n = read(mem_pipe_fd[0], &buf, sizeof(buf));
    if (n != sizeof(buf))
        fprintf(stderr, "read_mem: short read (%zd)\n", n);

    return buf;
}

int main_loop(int wait_time)
{
    while (1)
    {
        MemData mem_data = read_mem();
        CpuData cpu_data = read_cpu();
        ProcData proc_data = read_proc();

        printf("\n========== System Status ==========\n");
        printf("--- Memory ---\n");
        print_mem_data(&mem_data);

        printf("--- CPU Load ---\n");
        print_cpu_data(&cpu_data);

        printf("--- Processes ---\n");
        print_proc_data(&proc_data);

        printf("============END============\n");
        fflush(stdout);

        sleep(1);
    }
}

int main(int argc, char *argv[])
{
    int wait_time = DEFAULT_INTERVAL_TIME;
    mkdir(OUTPUT_FOLDER, 0755);

    pipe(mem_pipe_fd);

    pid_mem_sensor = fork();
    if (pid_mem_sensor == 0)
    {
        // child mem_sensor
        close(mem_pipe_fd[0]);

        char fd_str[16];
        snprintf(fd_str, sizeof(fd_str), "%d", mem_pipe_fd[1]);

        char *memargv[] = {MEM_BIN_PATH, fd_str, NULL};
        execv(MEM_BIN_PATH, memargv);

        perror("execv failed, mem_sensor");
        exit(1);
    }
    else
    {
        if (mkfifo(CPU_FIFO_PATH, 0666) == -1 && errno != EEXIST)
        {
            perror("mkfifo failed");
            return 1;
        }

        // parent
        pid_cpu_sensor = fork();

        if (pid_cpu_sensor == 0)
        {
            // child cpu_sensor

            char *cpuargv[] = {CPU_BIN_PATH, NULL};
            execv(CPU_BIN_PATH, cpuargv);

            perror("execv failed, cpu_sensor");
            exit(1);
        }
        else
        {
            // parent
            if (mkfifo(PROC_FIFO_PATH, 0666) == -1 && errno != EEXIST)
            {
                perror("mkfifo failed");
                return 1;
            }

            pid_proc_sensor = fork();
            if (pid_proc_sensor == 0)
            {
                // child proc_sensor
                char *cpuargv[] = {PROC_BIN_PATH, NULL};
                execv(PROC_BIN_PATH, cpuargv);

                perror("execv failed, proc_sensor");
                exit(1);
            }
            else
            {
                // parent
                close(mem_pipe_fd[1]);
                mem_pipe_fd[1] = -1;

                install_signal_handlers();

                // execute loop
                main_loop(wait_time);

                close(mem_pipe_fd[0]);
            }
        }
    }
}