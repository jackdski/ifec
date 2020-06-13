/*
 * src_timers.c
 *
 *  Created on: Jan 4, 2020
 *      Author: jack
 */

#include <stdint.h>
#include <stdbool.h>

#include "src_timers.h"
#include "src_epwm.h"
#include "driverlib.h"
#include "device.h"

//static bool light_status = 1;
//static bool direction = 1;
//static bool system_active = false;

/** control loop active variables */
static bool pid_active = false;
static bool mppt_active = false;


// uint32_t period: [uS]
void init_timer(uint32_t timer_base, uint32_t period) {
    // enable clock
    switch(timer_base) {
        case(CPUTIMER0_BASE): SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TIMER0);  break;
        case(CPUTIMER1_BASE): SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TIMER1); break;
        case(CPUTIMER2_BASE): SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TIMER2); break;
    }

    uint32_t timer_count;
//    uint32_t clock_source_freq = SysCtl_getClock(DEVICE_OSCSRC_FREQ);   // 100MHz or 10ns period
    timer_count = (period * 100);// / clock_source_freq;

//    CPUTimer_selectClockSource(timer_base, CPUTIMER_CLOCK_SOURCE_SYS, CPUTIMER_CLOCK_PRESCALER_1);
    CPUTimer_setPeriod(timer_base, timer_count);
    CPUTimer_setPreScaler(timer_base, 0);
    CPUTimer_stopTimer(timer_base);
    CPUTimer_reloadTimerCounter(timer_base);
    CPUTimer_setEmulationMode(timer_base, CPUTIMER_EMULATIONMODE_RUNFREE);
    CPUTimer_enableInterrupt(timer_base);
}

/**********************************************************
 *                  S E T S / G E T S
 **********************************************************/

bool get_pid_active(void) {
    return pid_active;
}

void set_pid_active(bool state) {
    pid_active = state;
}

bool get_mppt_active(void) {
    return mppt_active;
}

void set_mppt_active(bool state) {
    mppt_active = state;
}

__interrupt void cpuTimer0ISR(void) {
    CPUTimer_reloadTimerCounter(CPUTIMER0_BASE);

    // Acknowledge this interrupt to receive more interrupts from group 1
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
    CPUTimer_startTimer(CPUTIMER0_BASE);
}

/**
 *  Sets off PID control loops
 */
__interrupt void PID_Timer_ISR(void) {
#ifdef LED_BRIGHTNESS
    // change light brightness
    if(direction) {
        change_pwm_duty_cycle(get_duty_cycle() + 0.5);
        if(get_duty_cycle() == 20.0) direction = 0;
    }
    else {
        change_pwm_duty_cycle(get_duty_cycle() - 0.5);
        if(get_duty_cycle() == 0.0) direction = 1;
    }
#endif
    pid_active = true;
}

/**
 *  Sets off MPPT control loops
 */
__interrupt void MPPT_Timer_ISR(void) {
    mppt_active = true;
}
