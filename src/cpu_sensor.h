#ifndef CPU_SENSOR_H
#define CPU_SENSOR_H

struct Cpu_Data
{
    float min1_avg;
    float min5_avg;
    float min15_avg;
};

void cpu_worker_thread(void *arg);

#endif // CPU_SENSOR_H