#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* Initialize the system timer */
void timer_init(uint32_t frequency);

/* Get the current tick count */
uint32_t timer_get_ticks(void);

/* Sleep for a specified number of ticks */
void timer_sleep(uint32_t ticks);

#endif /* TIMER_H */
