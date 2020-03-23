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

//#define USE_PID

#define TIMER_250US     250
#define TIMER_500US     500
#define TIMER_10MS      10000
#define TIMER_500MS     500000


uint16_t status;

/** Global Variables */
uint16_t status;
PID_t buck_one_pid;
PID_t buck_two_pid;
MPPT_t mppt_one;
MPPT_t mppt_two;


// main
void main(void) {
    // Initialize device clock and peripherals
    Device_init();
    Device_initGPIO();

    // PID
    PID_init(&buck_one_pid, 3.2, 2.1, 2.3, 1.20, 1); // 10ms intervals, bucks ~20V to 12V with 1k and 330ohm voltage divider
    PID_init(&buck_two_pid, 3.2, 2.1, 2.3, 1.20, 1); // 10ms intervals, bucks ~20V to 12V with 1k and 330ohm voltage divider

    // MPPT
    mppt_init(&mppt_one, MPPT1_BASE, 0.1, 5.0);
    mppt_init(&mppt_two, MPPT2_BASE, 2.5, 5.0);

    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    Interrupt_initModule();

    // Initialize the PIE vector table with pointers to the shell Interrupt Service Routines (ISR).
    Interrupt_initVectorTable();
    Interrupt_register(INT_ADCA1, &adc_buck_one_irq);
    Interrupt_register(INT_ADCA2, &adc_battery_v_irq);
    Interrupt_register(INT_ADCA3, &adc_mppt_one_v_irq);
    Interrupt_register(INT_ADCA4, &adc_mppt_one_i_irq);

//    Interrupt_register(INT_TIMER0, &cpuTimer0ISR);
    Interrupt_register(INT_TIMER1, &cpuTimer1ISR);
    Interrupt_register(INT_TIMER2, &cpuTimer2ISR);


    // Configure peripherals
    init_led5();
    init_adc();
    initEPWMGPIO();

    initEPWM(BUCK3V3_BASE);
    initEPWM(BUCK5V0_BASE);
    change_pwm_duty_cycle(BUCK3V3_BASE, 0.0);
    change_pwm_duty_cycle(BUCK5V0_BASE, 0.0);

    initEPWM(MPPT1_BASE);
    initEPWM(MPPT2_BASE);
    change_pwm_duty_cycle(MPPT1_BASE, 0.0);
    change_pwm_duty_cycle(MPPT2_BASE, 0.0);

//    init_timer(CPUTIMER0_BASE, 500000);
//    init_timer(CPUTIMER1_BASE, TIMER_500MS);
    init_timer(CPUTIMER1_BASE, TIMER_250US);
    init_timer(CPUTIMER2_BASE, TIMER_500US);


    // Enable interrupts
    Interrupt_enable(INT_ADCA1);
    Interrupt_enable(INT_ADCA2);
    Interrupt_enable(INT_ADCA3);
    Interrupt_enable(INT_ADCA4);
    //    Interrupt_enable(INT_TIMER0);
    Interrupt_enable(INT_TIMER1);
    Interrupt_enable(INT_TIMER2);

//    CPUTimer_startTimer(CPUTIMER0_BASE);
    CPUTimer_startTimer(CPUTIMER1_BASE);
    CPUTimer_startTimer(CPUTIMER2_BASE);

    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    EINT;
    ERTM;

    // Loop Forever
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
        }
        else if ((get_pid_active() == false) && (get_mppt_active() == false)) {  // if everything is ready to go
            IDLE;   // go to low-power mode until TIMER1 wakes up CPU
        }
#endif
#ifdef TEST_OUTPUT_BUCKS
        if(get_pid_active() == true) {
            change_pwm_duty_cycle(BUCK3V3_BASE, PID_calculate(&buck_one_pid, get_buck_v(BUCK3V3_BASE)));
            change_pwm_duty_cycle(BUCK5V0_BASE, PID_calculate(&buck_two_pid, get_buck_v(BUCK5V0_BASE)));
            set_pid_active(false);
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
        }
        else if (get_mppt_active() == false) {  // if everything is ready to go
            IDLE;   // go to low-power mode until TIMER1 wakes up CPU
        }
#endif
    }
}
