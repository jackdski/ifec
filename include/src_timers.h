/*
 * src_timers.h
 *
 *  Created on: Jan 4, 2020
 *      Author: jack
 */

#ifndef INCLUDE_SRC_TIMERS_H_
#define INCLUDE_SRC_TIMERS_H_

void init_timer(uint32_t timer_base, uint32_t period);
bool get_system_active(void);
void set_system_active(bool state);

__interrupt void cpuTimer0ISR(void);
__interrupt void cpuTimer1ISR(void);
__interrupt void cpuTimer2ISR(void);

#endif /* INCLUDE_SRC_TIMERS_H_ */
