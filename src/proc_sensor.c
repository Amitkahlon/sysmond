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

double curr_elapsed_time = 0;

int is_numeric(const char *str) {
    if (*str == '\0') return 0;
    while (*str) {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

int extract_active_procs_count() {
    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        perror("Failed to open /proc directory");
        return -1;
    }

    int process_count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (is_numeric(entry->d_name)) {
            process_count++;
        }
    }

    closedir(dir);
    return process_count;
}

void write_to_file(const int proc_count, char output_path[]) {
    FILE *output_fd = fopen(output_path, "w");

    if (output_fd == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    fprintf(output_fd, "active proc count: %d \n", proc_count);

    fclose(output_fd);

}

int main(int argc, char *argv[])
{
    struct timespec prev_time, curr_time; // to get the actual elapsed time.
    int waitTime = DEFAULT_WAIT_TIME;
    char output_path[256];
    bool silent = false;
    int proc_count = 0;

    snprintf(output_path, sizeof(output_path), "%s/%s", OUTPUT_FOLDER, PROC_OUTPUT_FILE);

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

        proc_count = extract_active_procs_count();

        if(!silent) {
            printf("proc count: %d\n", proc_count);
        }

        write_to_file(proc_count, output_path);

        prev_time = curr_time;
    }
}