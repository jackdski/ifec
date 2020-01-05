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

static uint32_t cpuTimer0IntCount = 0;
static uint32_t cpuTimer1IntCount = 0;
static uint32_t cpuTimer2IntCount = 0;

static bool light_status = 1;
static bool direction = 1;
static bool system_active = false;


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

    if (timer_base == CPUTIMER0_BASE) {
        cpuTimer0IntCount = 0;
    }
    else if(timer_base == CPUTIMER1_BASE) {
        cpuTimer1IntCount = 0;
    }
    else if(timer_base == CPUTIMER2_BASE) {
        cpuTimer2IntCount = 0;
    }
}

bool get_system_active(void) {
    return system_active;
}

void set_system_active(bool state) {
    system_active = state;
}

__interrupt void cpuTimer0ISR(void) {
    cpuTimer0IntCount++;
    CPUTimer_reloadTimerCounter(CPUTIMER0_BASE);


    // Acknowledge this interrupt to receive more interrupts from group 1
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
    CPUTimer_startTimer(CPUTIMER0_BASE);
}

__interrupt void cpuTimer1ISR(void) {
    cpuTimer1IntCount++;
    GPIO_writePin(DEVICE_GPIO_PIN_LED1, light_status);  // Toggle LED
    light_status ^= 1;

    // change light brightness
//    if(direction) {
//        change_pwm_duty_cycle(get_duty_cycle() + 0.5);
//        if(get_duty_cycle() == 20.0) direction = 0;
//    }
//    else {
//        change_pwm_duty_cycle(get_duty_cycle() - 0.5);
//        if(get_duty_cycle() == 0.0) direction = 1;
//    }

    system_active = true;
}

__interrupt void cpuTimer2ISR(void) {
    cpuTimer2IntCount++;
}
