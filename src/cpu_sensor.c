#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include "config.h"
#include "sysmon_ipc.h"
#include <errno.h>
#include <fcntl.h>


FILE *get_cpu_output_fd()
{
    return fopen(CPU_OUTPUT_PATH, "w");
}

FILE *get_cpu_input_fd()
{
    return fopen(LOAD_AVG_PATH, "r");
}

CpuData extract_load_avg_info()
{
    CpuData cpu_data = {0};
    FILE *mem_info_fd = get_cpu_input_fd();

    if (mem_info_fd == NULL)
    {
        perror("cpu_sensor: Failed to open: ");
        perror(LOAD_AVG_PATH);

        return cpu_data;
    }

    if (fscanf(mem_info_fd, "%lf %lf %lf", &cpu_data.load1, &cpu_data.load5, &cpu_data.load15) != 3)
    {
        perror("cpu_sensor: Error reading the first three values.\n");
    }

    fclose(mem_info_fd);

    return cpu_data;
}

int write_to_output(const CpuData *cpu_data)
{
    if (mkfifo(CPU_FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("cpu_sensor: mkfifo failed");
        return 1;
    }


    int fd = open(CPU_FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("cpu_sensor: open for write failed");
        return 1;
    }
    
    ssize_t written = write(fd, cpu_data, sizeof(*cpu_data));
    if (written == -1) {
        perror("cpu_sensor: write failed");
        close(fd);
        return 1;
    }
    
    close(fd);
    return 0;
}

int main(int argc, char *argv[])
{
    CpuData cpu_data;
    int wait_time = DEFAULT_INTERVAL_TIME;

    while (1)
    {
        cpu_data = extract_load_avg_info();

        write_to_output(&cpu_data);
  
        sleep(wait_time);
    }
}