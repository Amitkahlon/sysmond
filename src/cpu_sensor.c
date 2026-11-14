#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include "config.h"

double curr_elapsed_time = 0;

struct Cpu_Data
{
    float min1_avg;
    float min5_avg;
    float min15_avg;
};

void extract_load_avg_info(struct Cpu_Data *cpu_data)
{
    FILE *mem_info_fd = fopen(LOAD_AVG_PATH, "r");

    if (mem_info_fd == NULL)
    {
        perror("Failed to open: ");
        perror(LOAD_AVG_PATH);

        return;
    }

    if (fscanf(mem_info_fd, "%f %f %f", &cpu_data->min1_avg, &cpu_data->min5_avg, &cpu_data->min15_avg) != 3)
    {
        perror("Error reading the first three values.\n");
    }

    fclose(mem_info_fd);
}

void print(const struct Cpu_Data cpu_data)
{
    printf("First:  %.2f\n", cpu_data.min1_avg);
    printf("Second: %.2f\n", cpu_data.min5_avg);
    printf("Third:  %.2f\n", cpu_data.min15_avg);
}


void write_to_file(const struct Cpu_Data *cpu_data, char output_path[])
{
    FILE *output_fd = fopen(output_path, "w");

    if (output_fd == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    fprintf(output_fd, "1 Minute Averge: %f kB\n", cpu_data->min1_avg);
    fprintf(output_fd, "5 Minute Averge: %f kB\n", cpu_data->min5_avg);
    fprintf(output_fd, "15 Minute Averge: %f kB\n", cpu_data->min15_avg);
    fprintf(output_fd,"Actual time elapsed: %.6f seconds\n", curr_elapsed_time);

    fclose(output_fd);
}

int main(int argc, char *argv[])
{
    struct Cpu_Data cpu_data;
    struct timespec prev_time, curr_time; // to get the actual elapsed time.
    int waitTime = DEFAULT_WAIT_TIME;
    char output_path[256];
    bool silent = false;

    snprintf(output_path, sizeof(output_path), "%s/%s", OUTPUT_FOLDER, CPU_OUTPUT_FILE);

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

        extract_load_avg_info(&cpu_data);

        if(!silent) {
            print(cpu_data);
        }

        write_to_file(&cpu_data, output_path);

        prev_time = curr_time;
    }
}