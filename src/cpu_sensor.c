#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include "config.h"
#include "cpu_sensor.h"
#include "system_state.h"


/*deprecated from part 1*/
FILE* get_cpu_output_fd(){
    return fopen(CPU_OUTPUT_PATH, "w");
}

FILE* get_cpu_input_fd(){
    return fopen(LOAD_AVG_PATH, "r");
}

struct Cpu_Data extract_load_avg_info()
{
    struct Cpu_Data cpu_data = {0};
    FILE *mem_info_fd = get_cpu_input_fd();

    if (mem_info_fd == NULL)
    {
        perror("Failed to open " LOAD_AVG_PATH);

        return cpu_data;
    }

    if (fscanf(mem_info_fd, "%f %f %f", &cpu_data.min1_avg, &cpu_data.min5_avg, &cpu_data.min15_avg) != 3)
    {
        fprintf(stderr, "Error reading the first three values from %s\n", LOAD_AVG_PATH);
    }

    fclose(mem_info_fd);


    return cpu_data;
}

void print(const struct Cpu_Data *cpu_data)
{
    printf("First:  %.2f\n", cpu_data->min1_avg);
    printf("Second: %.2f\n", cpu_data->min5_avg);
    printf("Third:  %.2f\n", cpu_data->min15_avg);
}

/*deprecated from part 1*/
static void write_to_file(const struct Cpu_Data *cpu_data, char output_path[])
{
    FILE *output_fd = get_cpu_output_fd();

    if (output_fd == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    fprintf(output_fd, "1 Minute Averge: %f\n", cpu_data->min1_avg);
    fprintf(output_fd, "5 Minute Averge: %f\n", cpu_data->min5_avg);
    fprintf(output_fd, "15 Minute Averge: %f\n", cpu_data->min15_avg);

    fclose(output_fd);
}


static void update_system_state(const struct Cpu_Data *cpu_data) {
    system_cpu_set(cpu_data->min1_avg, cpu_data->min5_avg, cpu_data->min15_avg);
}

void *cpu_worker_thread(void *arg)
{
    struct Cpu_Data cpu_data;
    int sample_interval_sec = DEFAULT_SAMPLE_INTERVAL_SEC;

    int int_arg = (int)(intptr_t)arg;
    if (int_arg > 0)
    {
        sample_interval_sec = int_arg;
    }


    printf("cpu_sensor: Sampling interval is set to: %d seconds\n", sample_interval_sec);


    while (1)
    {
        cpu_data = extract_load_avg_info();

        update_system_state(&cpu_data);

        sleep(sample_interval_sec);
    }

    return NULL;
}