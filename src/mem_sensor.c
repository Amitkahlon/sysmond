#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include "config.h"

double curr_elapsed_time = 0;


struct Mem_Data
{
    unsigned long mem_total;
    unsigned long mem_free;
    unsigned long mem_avaliable;
    unsigned long swap_total;
    unsigned long swap_free;
};

void extract_mem_info(struct Mem_Data *mem_data)
{
    char line[256];
    unsigned long tmp_value = 0;

    FILE *mem_info_fd = fopen(MEM_INFO_PATH, "r");

    if (mem_info_fd == NULL)
    {
        perror("Failed to open /proc/meminfo");
        return;
    }

    while (fgets(line, sizeof(line), mem_info_fd) != NULL)
    {
        if (sscanf(line, "MemTotal: %lu kB", &tmp_value) == 1)
        {
            mem_data->mem_total = tmp_value;
        }

        else if (sscanf(line, "MemFree: %lu kB", &tmp_value) == 1)
        {
            mem_data->mem_free = tmp_value;
        }

        else if (sscanf(line, "MemAvailable: %lu kB", &tmp_value) == 1)
        {
            mem_data->mem_avaliable = tmp_value;
        }

        else if (sscanf(line, "SwapTotal: %lu kB", &tmp_value) == 1)
        {
            mem_data->swap_total = tmp_value;
        }

        else if (sscanf(line, "SwapFree: %lu kB", &tmp_value) == 1)
        {
            mem_data->swap_free = tmp_value;
        }
    }

    fclose(mem_info_fd);
}

void print_mem_data(const struct Mem_Data *mem_data)
{
    printf("Actual time elapsed: %.6f seconds\n", curr_elapsed_time);
    printf("MemTotal: %lu kB\n", mem_data->mem_total);
    printf("MemFree: %lu kB\n", mem_data->mem_free);
    printf("MemAvailable: %lu kB\n", mem_data->mem_avaliable);
    printf("SwapTotal: %lu kB\n", mem_data->swap_total);
    printf("SwapFree: %lu kB\n", mem_data->swap_free);
}

void write_to_file(const struct Mem_Data *mem_data, char output_path[])
{
    FILE *output_fd = fopen(output_path, "w");

    if (output_fd == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    fprintf(output_fd, "=== Memory Information ===\n");
    fprintf(output_fd,"Actual time elapsed: %.6f seconds\n", curr_elapsed_time);
    fprintf(output_fd, "Mem Total:     %lu kB\n", mem_data->mem_total);
    fprintf(output_fd, "Mem Free:     %lu kB\n", mem_data->mem_total);
    fprintf(output_fd, "Mem Available:     %lu kB\n", mem_data->mem_total);
    fprintf(output_fd, "Swap Total:     %lu kB\n", mem_data->mem_total);
    fprintf(output_fd, "Swap Free:     %lu kB\n", mem_data->mem_total);
    fprintf(output_fd, "==========================");

    fclose(output_fd);
}




int main(int argc, char *argv[])
{
    struct timespec prev_time, curr_time; //to get the actual elapsed time.
    struct Mem_Data mem_data;
    int waitTime = DEFAULT_WAIT_TIME;
    char output_path[256];
    bool silent = false;
    FILE *mem_info_fd = NULL;
    FILE *output_fd = NULL;

    snprintf(output_path, sizeof(output_path), "%s/%s", OUTPUT_FOLDER, MEM_OUTPUT_FILE);

    if (argc > 1)
    {
        waitTime = atoi(argv[1]);
    }

    if (argc > 2 && strcmp(argv[2], "silent") == 0)
    {
        silent = true;
    }

    
    printf("Wait time between each check is set to: %d seconds\n", waitTime);

    clock_gettime(CLOCK_MONOTONIC, &prev_time);

    while (1)
    {
        clock_gettime(CLOCK_MONOTONIC, &curr_time);

        sleep(waitTime);

        curr_elapsed_time = (curr_time.tv_sec - prev_time.tv_sec) +
                         (curr_time.tv_nsec - prev_time.tv_nsec) / 1e9;


        // ensures that sysmon foldr exists in the tmp directory
        mkdir(OUTPUT_FOLDER, 0755);

        extract_mem_info(&mem_data);

        if (!silent)
        {
            print_mem_data(&mem_data);
        }

        write_to_file(&mem_data, output_path);

        prev_time = curr_time;
    }

    

    //finally
    fclose(mem_info_fd);
    fclose(output_fd);


    return 0;
}