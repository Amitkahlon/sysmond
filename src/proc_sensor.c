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
#include "system_state.h"

/*deprecated from part 1*/
FILE *get_proc_output_fd()
{
    return fopen(PROC_OUTPUT_PATH, "w");
}

DIR *get_proc_input_fd()
{
    return opendir(PROC_FOLDER_PATH);
}

int is_numeric(const char *str)
{
    if (*str == '\0')
        return 0;
    while (*str)
    {
        if (!isdigit(*str))
        {
            return 0;
        }
        str++;
    }
    return 1;
}

int extract_active_procs_count()
{
    DIR *dir = get_proc_input_fd();

    if (dir == NULL)
    {
        perror("Failed to open /proc directory");
        return -1;
    }

    int process_count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (is_numeric(entry->d_name))
        {
            process_count++;
        }
    }

    closedir(dir);
    return process_count;
}

/*deprecated from part 1*/
static void write_to_file(const int proc_count, char output_path[])
{
    FILE *output_fd = get_proc_output_fd();

    if (output_fd == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    fprintf(output_fd, "active proc count: %d \n", proc_count);

    fclose(output_fd);
}

static void update_system_state(int proc_count)
{
    system_proc_set(proc_count);
}

void proc_worker_thread(void *arg)
{
    int wait_time = DEFAULT_WAIT_TIME;
    int proc_count;

    int int_arg = (int)arg;
    if (int_arg > 0)
    {
        wait_time = int_arg;
    }

    printf("proc_sensor: Wait time between each check is set to: %d seconds\n", wait_time);

    while (1)
    {
        sleep(wait_time);

        proc_count = extract_active_procs_count();

        update_system_state(proc_count);
    }
}