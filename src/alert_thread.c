#include "system_state.h"
#include <stdio.h>


void *alert_worker_thread(void *arg) {
    float THRESHOLD = 1.50;

    while(1) {
        system_wait_for_high_load(THRESHOLD);
    }

    return NULL;

    
}