#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <poll.h>
#include <time.h>
#include "config.h"
#include "ui.h"

/* ANSI escape codes */
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define DIM     "\033[2m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define CYAN    "\033[36m"

#define CLEAR_SCREEN  "\033[H\033[2J"   /* cursor to top-left, clear screen */
#define ALT_SCREEN_ON "\033[?1049h"     /* separate screen, like vim/top    */
#define ALT_SCREEN_OFF "\033[?1049l"
#define CURSOR_HIDE   "\033[?25l"
#define CURSOR_SHOW   "\033[?25h"

#define BAR_WIDTH 16
#define LINE "─────────────────────────────────────────────────────────────"

static struct termios s_orig_termios;
static int s_raw_mode = 0;
static int s_active = 0;

void ui_restore(void)
{
    if (!s_active)
        return;

    printf(CURSOR_SHOW ALT_SCREEN_OFF);
    fflush(stdout);

    if (s_raw_mode)
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &s_orig_termios);

    s_raw_mode = 0;
    s_active = 0;
}

void ui_init(void)
{
    /* Raw mode: keys arrive one at a time, without Enter and without echo.
       ISIG is off too, so Ctrl+C arrives as a key (3) and quits cleanly. */
    if (isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &s_orig_termios) == 0)
    {
        struct termios raw = s_orig_termios;
        raw.c_lflag &= ~(ICANON | ECHO | ISIG);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        s_raw_mode = 1;
    }

    printf(ALT_SCREEN_ON CURSOR_HIDE);
    fflush(stdout);

    s_active = 1;
    atexit(ui_restore);
}

int ui_wait_key(int timeout_ms)
{
    /* No keyboard (e.g. input redirected): just wait. */
    if (!s_raw_mode)
    {
        sleep(timeout_ms / 1000);
        return 0;
    }

    struct pollfd pfd = { .fd = STDIN_FILENO, .events = POLLIN };

    if (poll(&pfd, 1, timeout_ms) > 0)
    {
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1)
            return c;
    }

    return 0;
}

static double percent(long part, long total)
{
    return total > 0 ? 100.0 * part / total : 0.0;
}

static double kb_to_gb(long kb)
{
    return kb / (1024.0 * 1024.0);
}

static void draw_bar(double pct)
{
    int filled = (int)(pct * BAR_WIDTH / 100.0 + 0.5);

    printf("[" CYAN);
    for (int i = 0; i < BAR_WIDTH; i++)
        printf(i < filled ? "█" : "░");
    printf(RESET "] %3.0f%%", pct);
}

void ui_draw(const SystemState *s, int sample_interval_sec)
{
    char timestamp[20];
    time_t now = time(NULL);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    printf(CLEAR_SCREEN);

    /* Header */
    printf(BOLD " sysmond " RESET DIM "─────────────────────────────── " RESET "%s\n", timestamp);

    /* Memory + swap */
    printf(BOLD "  MEMORY   " RESET "Total %4.1f GB   Avail %4.1f GB   ",
           kb_to_gb(s->mem_total), kb_to_gb(s->mem_avail));
    draw_bar(percent(s->mem_total - s->mem_avail, s->mem_total));
    printf("\n");

    printf("           Swap  %4.1f GB    Free  %4.1f GB   ",
           kb_to_gb(s->swap_total), kb_to_gb(s->swap_free));
    draw_bar(percent(s->swap_total - s->swap_free, s->swap_total));
    printf("\n");

    /* CPU + processes */
    printf(BOLD "  CPU      " RESET "1m %.2f   5m %.2f   15m %.2f\n", s->load1, s->load5, s->load15);
    printf(BOLD "  PROCS    " RESET "%d\n", s->total_procs);

    /* Alert */
    printf(DIM " " LINE RESET "\n");
    if (s->load1 > ALERT_THRESHOLD)
        printf(BOLD RED "  ⚠ ALERT  load %.2f > %.2f" RESET "\n", s->load1, ALERT_THRESHOLD);
    else
        printf(GREEN "  ✓ OK     load %.2f <= %.2f" RESET "\n", s->load1, ALERT_THRESHOLD);
    printf(DIM " " LINE RESET "\n");

    /* Footer */
    printf(DIM "  q: quit    sampling: %ds" RESET "\n", sample_interval_sec);

    fflush(stdout);
}
