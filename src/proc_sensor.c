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
#include <fcntl.h>
#include "sysmon_ipc.h"



#define FIFO_PATH "/tmp/sysmon_proc.fifo"


FILE* get_proc_output_fd(){
    return fopen(PROC_OUTPUT_PATH, "w");
}

DIR* get_proc_input_fd(){
    return opendir(PROC_FOLDER_PATH);
}

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

ProcData extract_active_procs_count() {
    ProcData proc_data = {0};  

    DIR *dir = get_proc_input_fd();
    
    if (dir == NULL) {
        perror("proc_sensor: Failed to open /proc directory");
        return proc_data;
    }

    int process_count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if (is_numeric(entry->d_name)) {
            process_count++;
        }
    }

    proc_data.total_procs = process_count;

    closedir(dir);
    return proc_data;
}

int write_to_output(const ProcData proc_data) {
    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("cpu_sensor: mkfifo failed");
        return 1;
    }

    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("cpu_sensor: open for write failed");
        return 1;
    }
    
    ssize_t written = write(fd, &proc_data, sizeof(proc_data));
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
    int waitTime = DEFAULT_INTERVAL_TIME;
    ProcData proc_data;

    while (1)
    {
        proc_data = extract_active_procs_count();

        write_to_output(proc_data);

        sleep(waitTime);
    }
}