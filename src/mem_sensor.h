#ifndef MEM_SENSOR_H
#define MEM_SENSOR_H

struct Mem_Data
{
    unsigned long mem_total;
    unsigned long mem_free;
    unsigned long mem_avaliable;
    unsigned long swap_total;
    unsigned long swap_free;
};

void mem_worker_thread(void *arg);

#endif // MEM_SENSOR_H