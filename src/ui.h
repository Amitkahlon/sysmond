#ifndef UI_H
#define UI_H

#include "system_state.h"

/* Switch the terminal to the full-screen UI (raw keys, hidden cursor). */
void ui_init(void);

/* Put the terminal back to normal. Safe to call more than once. */
void ui_restore(void);

/* Redraw the whole screen from a snapshot. */
void ui_draw(const SystemState *state, int wait_time);

/* Wait up to timeout_ms for a key press. Returns the key, or 0 on timeout. */
int ui_wait_key(int timeout_ms);

#endif // UI_H
