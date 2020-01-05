//*****************************************************************************
//
// FILE:   main.c
//
// TITLE:  IFEC Project
//
// IFEC 2020 CU Boulder
//
//*****************************************************************************

#include <stdbool.h>

#include "driverlib.h"
#include "device.h"
#include "src_adc.h"
#include "src_epwm.h"
#include "src_gpio.h"
#include "src_timers.h"
#include "pid.h"

#define USE_PID

#define TIMER_10MS  10000
#define TIMER_500MS 500000


uint16_t status;
PID_t buck_one_pid;

// main
void main(void) {
    // Initialize device clock and peripherals
    Device_init();

    // PID
    // PID_init(&buck_one_pid, 3.2, 2.1, 2.3, 2.00, 10); // 10ms intervals, bucks 5V to 2.00V with 1k and 1k voltage divider
    PID_init(&buck_one_pid, 3.2, 2.1, 2.3, 1.250, 10); // 10ms intervals, bucks 8V to 5V with 1k and 330ohm voltage divider


    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    Interrupt_initModule();

    // Initialize the PIE vector table with pointers to the shell Interrupt Service Routines (ISR).
    Interrupt_initVectorTable();
    Interrupt_register(INT_ADCA1, &adc_buck_one_irq);
    Interrupt_register(INT_ADCA2, &adc_buck_two_irq);
    Interrupt_register(INT_ADCA3, &adc_mppt_one_irq);


//    Interrupt_register(INT_TIMER0, &cpuTimer0ISR);
    Interrupt_register(INT_TIMER1, &cpuTimer1ISR);
//    Interrupt_register(INT_TIMER2, &cpuTimer2ISR);


    // Configure peripherals
    Device_initGPIO();
    init_led5();
    init_adc();
    initEPWMGPIO();
    initEPWM1();
    change_pwm_duty_cycle(0.0);

//    init_timer(CPUTIMER0_BASE, 500000);
//    init_timer(CPUTIMER1_BASE, TIMER_500MS);
    init_timer(CPUTIMER1_BASE, TIMER_10MS);
//    init_timer(CPUTIMER1_BASE, 500000);


    // Enable interrupts
//    Interrupt_enable(INT_EPWM1);
    Interrupt_enable(INT_ADCA1);
//    Interrupt_enable(INT_TIMER0);
    Interrupt_enable(INT_TIMER1);
//    Interrupt_enable(INT_TIMER2);

//    CPUTimer_startTimer(CPUTIMER0_BASE);
    CPUTimer_startTimer(CPUTIMER1_BASE);
//    CPUTimer_startTimer(CPUTIMER2_BASE);

    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    EINT;
    ERTM;

    change_pwm_duty_cycle(25.0);

    // Loop Forever
    for(;;) {
#ifdef USE_PID
        if(get_system_active() == true) {
            change_pwm_duty_cycle(PID_calculate(&buck_one_pid, get_buck_one_v()));
            set_system_active(false);
        }
        else if (get_system_active() == false) {  // if everything is ready to go
               IDLE;
        }
#else

#endif
    }
}

