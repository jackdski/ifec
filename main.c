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

/** Controls */
#include "pid.h"
#include "mppt.h"

#define USE_PID

#define TIMER_10MS      10000
#define TIMER_500MS     500000
#define TIMER_1000MS    1000000

/** Global Variables */
uint16_t status;
PID_t buck_one_pid;
PID_t buck_two_pid;
PID_t mppt_one_pid;
PID_t mppt_two_pid;
MPPT_t mppt_one;
MPPT_t mppt_two;

// main
void main(void) {
    // Initialize device clock and peripherals
    Device_init();

    // PID
    PID_init(&buck_one_pid, 3.2, 2.1, 2.3, 1.250, 10); // 10ms intervals, bucks 8V to 5V with 1k and 330ohm voltage divider
    PID_init(&buck_two_pid, 3.2, 2.1, 2.3, 2.00, 10); // 10ms intervals, bucks 5V to 2.00V with 1k and 1k voltage divider
    PID_init(&mppt_one_pid, 3.2, 2.1, 2.3, 1.250, 10); // 10ms intervals, bucks ~17V to 8.00V with 10k and 1.8k voltage divider
    PID_init(&mppt_two_pid, 3.2, 2.1, 2.3, 1.250, 10); // 10ms intervals, bucks ~17V to 8.00V with 10k and 1.8k voltage divider

    // MPPT
    mppt_init(&mppt_one, MPPT1_BASE, 2.5, 5.0);
    mppt_init(&mppt_two, MPPT2_BASE, 2.5, 5.0);

    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    Interrupt_initModule();

    // Initialize the PIE vector table with pointers to the shell Interrupt Service Routines (ISR).
    Interrupt_initVectorTable();
    Interrupt_register(INT_ADCA1, &adc_buck_one_irq);
    Interrupt_register(INT_ADCA2, &adc_buck_two_irq);
    Interrupt_register(INT_ADCA3, &adc_mppt_one_irq);
    Interrupt_register(INT_ADCA4, &adc_mppt_two_irq);
    Interrupt_register(INT_TIMER1, &cpuTimer1ISR);
    Interrupt_register(INT_TIMER1, &cpuTimer2ISR);

    // Configure peripherals
    Device_initGPIO();
    init_led5();
    init_voltage_adc();
    init_current_adc();
    init_timer(CPUTIMER1_BASE, TIMER_10MS);
    init_timer(CPUTIMER2_BASE, TIMER_1000MS);

    // Configure ePWM
    initEPWMGPIO();
    initEPWM(BUCK3V3_BASE);
    change_pwm_duty_cycle(BUCK3V3_BASE, 0.0);
    initEPWM(BUCK5V0_BASE);
    change_pwm_duty_cycle(BUCK5V0_BASE, 0.0);
    initEPWM(MPPT1_BASE);
    change_pwm_duty_cycle(MPPT1_BASE, 0.0);
    initEPWM(MPPT2_BASE);
    change_pwm_duty_cycle(MPPT2_BASE, 0.0);

    // Enable interrupts
    Interrupt_enable(INT_ADCA1);
    Interrupt_enable(INT_TIMER1);
    Interrupt_enable(INT_TIMER2);


    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    EINT;
    ERTM;

    CPUTimer_startTimer(CPUTIMER1_BASE);        // Start PID Timer
    CPUTimer_startTimer(CPUTIMER2_BASE);        // Start MPPT Timer
    change_pwm_duty_cycle(BUCK3V3_BASE, 25.0);  // Initial PWM is 25.0%
    change_pwm_duty_cycle(BUCK5V0_BASE, 25.0);  // Initial PWM is 25.0%
    change_pwm_duty_cycle(MPPT1_BASE, 25.0);    // Initial PWM is 25.0%
    change_pwm_duty_cycle(MPPT2_BASE, 25.0);    // Initial PWM is 25.0%


    // Loop Forever
    for(;;) {
#ifdef USE_PID
        if(get_pid_active() == true) {
            change_pwm_duty_cycle(BUCK3V3_BASE, PID_calculate(&buck_one_pid, get_buck_v(BUCK3V3_BASE)));
            change_pwm_duty_cycle(BUCK5V0_BASE, PID_calculate(&buck_two_pid, get_buck_v(BUCK5V0_BASE)));
            set_pid_active(false);
        }
        if(get_mppt_active() == true) {
            // Update values
            mppt_update_values(&mppt_one);
            mppt_update_values(&mppt_two);

            // change duty cycle accordingly
            change_pwm_duty_cycle(mppt_one.mppt_base, (get_duty_cycle(mppt_one.mppt_base) - mppt_calculate(&mppt_one)));
            change_pwm_duty_cycle(mppt_two.mppt_base, (get_duty_cycle(mppt_two.mppt_base) - mppt_calculate(&mppt_two)));

            set_mppt_active(false);
        }
        else if ((get_pid_active() == false) && (get_mppt_active() == false)) {  // if everything is ready to go
               IDLE;    // go to low-power mode until TIMER1 wakes up CPU
        }
#endif
    }
}

