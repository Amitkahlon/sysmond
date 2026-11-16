#include "system_state.h"
#include <stdio.h>
#include "config.h"


void *alert_worker_thread(void *arg) {
    const double THRESHOLD = ALERT_THRESHOLD;

    while(1) {
        system_wait_for_load_above(THRESHOLD);
        system_wait_for_load_below(THRESHOLD);
    }

    return NULL;

    
}