#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include "config.h"
#include "mem_sensor.h"
#include "system_state.h"

FILE *get_mem_input_fd()
{
    return fopen(MEM_INFO_PATH, "r");
}

/*deprecated from part 1*/
FILE *get_mem_output_fd()
{
    return fopen(MEM_OUTPUT_PATH, "w");
}

struct Mem_Data extract_mem_info()
{
    struct Mem_Data mem_data = {0};
    char line[256];
    unsigned long tmp_value = 0;

    FILE *mem_info_fd = get_mem_input_fd();

    if (mem_info_fd == NULL)
    {
        perror("Failed to open /proc/meminfo");
        return mem_data;
    }

    while (fgets(line, sizeof(line), mem_info_fd) != NULL)
    {
        if (sscanf(line, "MemTotal: %lu kB", &tmp_value) == 1)
        {
            mem_data.mem_total = tmp_value;
        }

        else if (sscanf(line, "MemFree: %lu kB", &tmp_value) == 1)
        {
            mem_data.mem_free = tmp_value;
        }

        else if (sscanf(line, "MemAvailable: %lu kB", &tmp_value) == 1)
        {
            mem_data.mem_avaliable = tmp_value;
        }

        else if (sscanf(line, "SwapTotal: %lu kB", &tmp_value) == 1)
        {
            mem_data.swap_total = tmp_value;
        }

        else if (sscanf(line, "SwapFree: %lu kB", &tmp_value) == 1)
        {
            mem_data.swap_free = tmp_value;
        }
    }

    fclose(mem_info_fd);

    return mem_data;
}

static void print_mem_data(const struct Mem_Data *mem_data)
{
    printf("MemTotal: %lu kB\n", mem_data->mem_total);
    printf("MemFree: %lu kB\n", mem_data->mem_free);
    printf("MemAvailable: %lu kB\n", mem_data->mem_avaliable);
    printf("SwapTotal: %lu kB\n", mem_data->swap_total);
    printf("SwapFree: %lu kB\n", mem_data->swap_free);
}

/*deprecated from part 1*/
static void write_to_file(const struct Mem_Data *mem_data, char output_path[])
{
    FILE *output_fd = get_mem_output_fd();

    if (output_fd == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    fprintf(output_fd, "=== Memory Information ===\n");
    fprintf(output_fd, "Mem Total:     %lu kB\n", mem_data->mem_total);
    fprintf(output_fd, "Mem Free:     %lu kB\n", mem_data->mem_free);
    fprintf(output_fd, "Mem Available:     %lu kB\n", mem_data->mem_avaliable);
    fprintf(output_fd, "Swap Total:     %lu kB\n", mem_data->swap_total);
    fprintf(output_fd, "Swap Free:     %lu kB\n", mem_data->swap_free);
    fprintf(output_fd, "==========================");

    fclose(output_fd);
}

static void update_system_state(const struct Mem_Data *mem_data)
{
    system_mem_set(mem_data->mem_total,
         mem_data->mem_free,
          mem_data->mem_avaliable,
           mem_data->swap_total,
            mem_data->swap_free);
}

void mem_worker_thread(void *arg)
{
    struct Mem_Data mem_data;
    int wait_time = DEFAULT_WAIT_TIME;

    int int_arg = (int)arg;
    if (int_arg > 0)
    {
        wait_time = int_arg;
    }

    printf("mem_sensor: Wait time between each check is set to: %d seconds\n", wait_time);

    while (1)
    {
        sleep(wait_time);

        mem_data = extract_mem_info();

        update_system_state(&mem_data);
    }

    return 0;
}