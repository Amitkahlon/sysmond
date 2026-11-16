#include "system_state.h"
#include <stdio.h>


void *alert_worker_thread(void *arg) {
    (void)arg;
    const double THRESHOLD = 1.50;

    while(1) {
        /* Sleeps until the load crosses above the threshold, alerts once... */
        system_wait_for_load_above(THRESHOLD);
        /* ...then sleeps until it drops back, so the next spike alerts again. */
        system_wait_for_load_below(THRESHOLD);
    }

    return NULL;

    
}