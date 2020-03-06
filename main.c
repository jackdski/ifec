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

/** Test Selection **/
//#define NORMAL_OPERATION
#define TEST_OUTPUT_BUCKS
//#define TEST_OUTPUT_BUCKS_OPEN_LOOP
//#define TEST_MPPT_BUCKS

#ifdef NORMAL_OPERATION
#define USE_PID
#define USE_MPPT
#endif
#ifdef TEST_OUTPUT_BUCKS
#define USE_PID
#endif
#ifdef TEST_MPPT_BUCKS
#define USE_MPPT
#endif


/**  Controls Timers  **/
#define TIMER_10MS      10000
#define TIMER_500MS     500000
#define TIMER_1000MS    1000000

/** Global Variables */
uint16_t status;
PID_t buck_one_pid;
PID_t buck_two_pid;
MPPT_t mppt_one;
MPPT_t mppt_two;


/** main **/
void main(void) {
    // Initialize device clock and peripherals
    Device_init();

    // PID
    PID_init(&buck_one_pid, 3.2, 2.1, 2.3, 1.45, 10); // 10ms intervals, bucks 8V to 5V with 1k and 330ohm voltage divider
//    PID_init(&buck_two_pid, 3.2, 2.1, 2.3, 2.00, 10); // 10ms intervals, bucks 5V to 2.00V with 1k and 1k voltage divider

    // MPPT
//    mppt_init(&mppt_one, MPPT1_BASE, 2.5, 5.0);
//    mppt_init(&mppt_two, MPPT2_BASE, 2.5, 5.0);

    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    Interrupt_initModule();

    // Initialize the PIE vector table with pointers to the shell Interrupt Service Routines (ISR).
    Interrupt_initVectorTable();
    Interrupt_register(INT_ADCA1, &adc_buck_one_irq);
    Interrupt_register(INT_ADCA2, &adc_buck_two_irq);
//    Interrupt_register(INT_ADCA3, &adc_battery_v_irq);
//    Interrupt_register(INT_ADCA4, &adc_battery_i_irq);

//    Interrupt_register(INT_ADCB1, &adc_mppt_one_i_irq);
//    Interrupt_register(INT_ADCB2, &adc_mppt_two_i_irq);
//    Interrupt_register(INT_ADCB3, &adc_mppt_one_v_irq);
//    Interrupt_register(INT_ADCB4, &adc_mppt_two_v_irq);

    Interrupt_register(INT_TIMER1, &cpuTimer1ISR);
//    Interrupt_register(INT_TIMER2, &cpuTimer2ISR);

    // Configure peripherals
    Device_initGPIO();
    init_led5();
    init_battery_output_bucks_adc();
//    init_mppt_adc();
    init_timer(CPUTIMER1_BASE, TIMER_10MS);
//    init_timer(CPUTIMER2_BASE, TIMER_1000MS);

    // Configure ePWM
    initEPWMGPIO();
    initEPWM(BUCK3V3_BASE);
    change_pwm_duty_cycle(BUCK3V3_BASE, 0.0);
//    initEPWM(BUCK5V0_BASE);
//    change_pwm_duty_cycle(BUCK5V0_BASE, 0.0);
//    initEPWM(MPPT1_BASE);
//    change_pwm_duty_cycle(MPPT1_BASE, 0.0);
//    initEPWM(MPPT2_BASE);
//    change_pwm_duty_cycle(MPPT2_BASE, 0.0);

    // Enable interrupts
    Interrupt_enable(INT_ADCA1);
//    Interrupt_enable(INT_ADCA2);
    Interrupt_enable(INT_TIMER1);   // PID
//    Interrupt_enable(INT_TIMER2);   // MPPT

    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    EINT;
    ERTM;


    // Start Off timers
#ifdef USE_PID
    CPUTimer_startTimer(CPUTIMER1_BASE);        // Start PID Timer
#endif
#ifdef USE_MPPT
    CPUTimer_startTimer(CPUTIMER2_BASE);        // Start MPPT Timer
#endif


    // Start off PWM
#ifndef TEST_OUTPUT_BUCKS_OPEN_LOOP
#ifdef NORMAL_OPERATION
    change_pwm_duty_cycle(BUCK3V3_BASE, 25.0);  // Initial PWM is 25.0%
    change_pwm_duty_cycle(BUCK5V0_BASE, 25.0);  // Initial PWM is 25.0%
    change_pwm_duty_cycle(MPPT1_BASE, 25.0);    // Initial PWM is 25.0%
    change_pwm_duty_cycle(MPPT2_BASE, 25.0);    // Initial PWM is 25.0%
#endif
#ifdef TEST_OUTPUT_BUCKS
    change_pwm_duty_cycle(BUCK3V3_BASE, 25.0);  // Initial PWM is 25.0%
    change_pwm_duty_cycle(BUCK5V0_BASE, 25.0);  // Initial PWM is 25.0%
#endif
#ifdef TEST_MPPT_BUCKS
    change_pwm_duty_cycle(MPPT1_BASE, 25.0);    // Initial PWM is 25.0%
    change_pwm_duty_cycle(MPPT2_BASE, 25.0);    // Initial PWM is 25.0%
#endif
#endif

    /** Make sure bucks and currents are readable **/
//    bool initialized_elements = false;
//    while(initialized_elements == false) {
//        initialized_elements = true;
//        if(!(get_force_buck_v(BUCK3V3_BASE) > 0.0)) {
//            initialized_elements = false;
//        }
//        if(!(get_force_buck_v(BUCK5V0_BASE) > 0.0)) {
//            initialized_elements = false;
//        }
//        if(!(get_force_mppt_v(MPPT1_BASE) > 0.0)) {
//            initialized_elements = false;
//        }
//        if(!(get_force_mppt_v(MPPT2_BASE) > 0.0)) {
//            initialized_elements = false;
//        }
//        if(!(get_force_mppt_i(MPPT1_BASE) > 0.0)) {
//            initialized_elements = false;
//        }
//        if(!(get_force_mppt_i(MPPT2_BASE) > 0.0)) {
//            initialized_elements = false;
//        }
//        if(!(get_force_battery_v() > 0.0)) {
//            initialized_elements = false;
//        }
//        if(!(get_force_battery_i() > 0.0)) {
//            initialized_elements = false;
//        }
//    }

    // Reconfigure the ADCs to start conversions on timer interrupts
//    reconfig_adc_for_timers();

    /** SuperLoop **/
    for(;;) {
#ifdef NORMAL_OPERATION
        /** PID **/
        if(get_pid_active() == true) {
            change_pwm_duty_cycle(BUCK3V3_BASE, PID_calculate(&buck_one_pid, get_buck_v(BUCK3V3_BASE)));
            change_pwm_duty_cycle(BUCK5V0_BASE, PID_calculate(&buck_two_pid, get_buck_v(BUCK5V0_BASE)));
            set_pid_active(false);
        }

        /** MPPT **/
        if(get_mppt_active() == true) {
            /** Update values **/
            mppt_update_values(&mppt_one);
            mppt_update_values(&mppt_two);

            /** MPPT 1 **/
            if(mppt_one.suspended == false) {
                change_pwm_duty_cycle(mppt_one.mppt_base, (get_duty_cycle(mppt_one.mppt_base) + mppt_calculate(&mppt_one)));
            }
            else if(mppt_one.suspended == true) {
                // see if PV voltage is higher suspended voltage reading plus hysteresis
                float latest_mppt_1 = get_force_mppt_v(mppt_one.mppt_base);
                  if(latest_mppt_1 - mppt_one.suspended_v >= 2.0) {
                      ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_CPU1_TINT2, ADC_CH_ADCIN6, 15);
                  }
            }

            /** MPPT 2 **/
            if(mppt_two.suspended == false) {
                change_pwm_duty_cycle(mppt_two.mppt_base, (get_duty_cycle(mppt_two.mppt_base) + mppt_calculate(&mppt_two)));
            }
            else if(mppt_two.suspended == true) {
                // see if PV voltage is higher suspended voltage reading plus hysteresis
                float latest_mppt_2 = get_force_mppt_v(mppt_two.mppt_base);
                if(latest_mppt_2 - mppt_two.suspended_v >= 2.0) {
                    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_CPU1_TINT2, ADC_CH_ADCIN7, 15);
                }
            }

            set_mppt_active(false);
            toggle_led();
        }
        else if ((get_pid_active() == false) && (get_mppt_active() == false)) {  // if everything is ready to go
            IDLE;   // go to low-power mode until TIMER1 wakes up CPU
        }
#endif
#ifdef TEST_OUTPUT_BUCKS
        if(get_pid_active() == true) {
            change_pwm_duty_cycle(BUCK3V3_BASE, PID_calculate(&buck_one_pid, get_buck_v(BUCK3V3_BASE)));
//            change_pwm_duty_cycle(BUCK5V0_BASE, PID_calculate(&buck_two_pid, get_buck_v(BUCK5V0_BASE)));
            set_pid_active(false);
            toggle_led();
        }
        else if (get_pid_active() == false) {  // if everything is ready to go
            IDLE;   // go to low-power mode until TIMER1 wakes up CPU
        }
#endif
#ifdef TEST_MPPT_BUCKS
        if(get_mppt_active() == true) {
            // Update values
            mppt_update_values(&mppt_one);
            mppt_update_values(&mppt_two);

            // change duty cycle accordingly
            change_pwm_duty_cycle(mppt_one.mppt_base, (get_duty_cycle(mppt_one.mppt_base) + mppt_calculate(&mppt_one)));
            change_pwm_duty_cycle(mppt_two.mppt_base, (get_duty_cycle(mppt_two.mppt_base) + mppt_calculate(&mppt_two)));
            set_mppt_active(false);
            toggle_led();
        }
        else if (get_mppt_active() == false) {  // if everything is ready to go
            IDLE;   // go to low-power mode until TIMER1 wakes up CPU
        }
#endif
    }
}

