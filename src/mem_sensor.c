#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include "config.h"
#include "inc/sysmon_ipc.h"
#include <assert.h>

FILE *get_mem_output_fd()
{
    return fopen(MEM_OUTPUT_PATH, "w");
}

FILE *get_mem_input_fd()
{
    return fopen(MEM_INFO_PATH, "r");
}

MemData extract_mem_info()
{
    MemData MemData = {0};
    char line[256];
    unsigned long tmp_value = 0;

    FILE *mem_info_fd = get_mem_input_fd();

    if (mem_info_fd == NULL)
    {
        perror("Failed to open /proc/meminfo");
        return MemData;
    }

    while (fgets(line, sizeof(line), mem_info_fd) != NULL)
    {
        if (sscanf(line, "MemTotal: %lu kB", &tmp_value) == 1)
        {
            MemData.mem_total = tmp_value;
        }

        else if (sscanf(line, "MemFree: %lu kB", &tmp_value) == 1)
        {
            MemData.mem_free = tmp_value;
        }

        else if (sscanf(line, "MemAvailable: %lu kB", &tmp_value) == 1)
        {
            MemData.mem_avail = tmp_value;
        }

        else if (sscanf(line, "SwapTotal: %lu kB", &tmp_value) == 1)
        {
            MemData.swap_total = tmp_value;
        }

        else if (sscanf(line, "SwapFree: %lu kB", &tmp_value) == 1)
        {
            MemData.swap_free = tmp_value;
        }
    }

    fclose(mem_info_fd);

    return MemData;
}

int write_to_output(const MemData *mem_data, int fd)
{
    size_t expected = sizeof(*mem_data);
    ssize_t bytes_written = write(fd, mem_data, expected);
    if (bytes_written != expected)
    {
        perror("write failed");
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    MemData mem_data;
    int wait_time = DEFAULT_INTERVAL_TIME;
    int output_fd = STDOUT_FILENO;

    if (argc > 1)
    {
        output_fd = atoi(argv[1]);
    }

    while (1)
    {
        mem_data = extract_mem_info();

        write_to_output(&mem_data, output_fd);

        sleep(wait_time);
    }

    return 0;
}



