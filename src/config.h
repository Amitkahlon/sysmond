// config.h
#ifndef CONFIG_H
#define CONFIG_H

// DEFAULTS
#define DEFAULT_SAMPLE_INTERVAL_SEC 5
#define ALERT_THRESHOLD 1.50

// INPUT
#define MEM_INFO_PATH "/proc/meminfo"
#define LOAD_AVG_PATH "/proc/loadavg"
#define PROC_FOLDER_PATH "/proc"

// OUTPUT
#define OUTPUT_FOLDER "/tmp/sysmon"
#define MEM_OUTPUT_FILE "mem.dat"
#define CPU_OUTPUT_FILE "cpu.dat"
#define PROC_OUTPUT_FILE "proc.dat"

#define MEM_OUTPUT_PATH  OUTPUT_FOLDER "/" MEM_OUTPUT_FILE
#define CPU_OUTPUT_PATH  OUTPUT_FOLDER "/" CPU_OUTPUT_FILE
#define PROC_OUTPUT_PATH OUTPUT_FOLDER "/" PROC_OUTPUT_FILE


#endif // CONFIG_H