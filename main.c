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
#include "config.h"

//#include "driverlib.h"
//#include "device.h"
#include "src_adc.h"
#include "src_epwm.h"
#include "src_gpio.h"
#include "src_timers.h"

/** Controls */
#include "pid.h"
#include "mppt.h"

/** Test Selection **/
/*      NORMAL_OPERATION
 *      TEST_OUTPUT_BUCKS
 *      TEST_OUTPUT_BUCKS_OPEN_LOOP
 *      TEST_MPPT_BUCKS
 **/
#define NORMAL_OPERATION 1


uint16_t status;

/** Global Variables */
uint16_t status;
PID_t five_volt_buck_pid;
PID_t three_volt_buck_pid;
MPPT_t mppt_one;
MPPT_t mppt_two;


// main
void main(void) {
    // Initialize device clock and peripherals
    Device_init();
    Device_initGPIO();

    // PID
    PID_init(&five_volt_buck_pid, KP, KI, KD, V_BUCK_5V_REF, PID_US);
    PID_init(&three_volt_buck_pid, KP, KI, KD, V_BUCK_3V3_REF, PID_US);

    // MPPT
    mppt_init(&mppt_one, MPPT_ONE_ID, MPPT_1_DELTA_DC, MPPT_1_DELTA_DC_MAX);
    mppt_init(&mppt_two, MPPT_TWO_ID, MPPT_2_DELTA_DC, MPPT_2_DELTA_DC_MAX);

    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    Interrupt_initModule();

    // Initialize the PIE vector table with pointers to the shell Interrupt Service Routines (ISR).
    Interrupt_initVectorTable();

    Interrupt_register(PID_TIMER_INT, &PID_Timer_ISR);
    Interrupt_register(MPPT_TIMER_INT, &MPPT_Timer_ISR);

    // Configure peripherals
    init_led5();
    init_adc();

    initEPWMGPIO();
    initEPWM(BUCK_5V_PWM);
    initEPWM(BUCK_3V3_PWM);
    initEPWM(MPPT_1_PWM);
    initEPWM(MPPT_2_PWM);

    init_timer(PID_TIMER, PID_US);
    init_timer(MPPT_TIMER, TIMER_500US);


    // Enable interrupts
    Interrupt_enable(INT_ADCA1);
    Interrupt_enable(INT_ADCA2);
    Interrupt_enable(INT_ADCA3);
    Interrupt_enable(INT_ADCA4);

    Interrupt_enable(PID_TIMER_INT);
    Interrupt_enable(MPPT_TIMER_INT);

    CPUTimer_startTimer(PID_TIMER);
    CPUTimer_startTimer(MPPT_TIMER);

    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    EINT;
    ERTM;

    // Loop Forever
    for(;;) {
#if defined(NORMAL_OPERATION)
        /** PID **/
        if(get_pid_active() == true) {
            change_pwm_duty_cycle(BUCK_5V_PWM, PID_calculate(&five_volt_buck_pid, get_buck_v(BUCK_5V_ID)));
            change_pwm_duty_cycle(BUCK_3V3_PWM, PID_calculate(&three_volt_buck_pid, get_buck_v(BUCK_3V3_ID)));
            set_pid_active(false);
        }

        /** MPPT **/
        if(get_mppt_active() == true) {
            /** Update values **/
            mppt_update_values(&mppt_one);
            mppt_update_values(&mppt_two);

            change_pwm_duty_cycle(MPPT_1_PWM, (get_duty_cycle(MPPT_1_PWM) + mppt_calculate(&mppt_one)));
            change_pwm_duty_cycle(MPPT_2_PWM, (get_duty_cycle(MPPT_2_PWM) + mppt_calculate(&mppt_two)));
            set_mppt_active(false);

            /* TODO: Implement ability to switch between two MPPT's/PV's
                if(mppt_one.suspended == false) {
                    change_pwm_duty_cycle(MPPT_1_PWM, (get_duty_cycle(MPPT_1_PWM) + mppt_calculate(&mppt_one)));
                }
                else if(mppt_one.suspended == true) {
                    // see if PV voltage is higher suspended voltage reading plus hysteresis
                    float latest_mppt_1 = get_force_mppt_v(mppt_one.mppt_base);
                      if(latest_mppt_1 - mppt_one.suspended_v >= 2.0) {
                          ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_CPU1_TINT2, ADC_CH_ADCIN6, 15);
                      }
                }

                if(mppt_two.suspended == false) {
                    change_pwm_duty_cycle(MPPT_2_PWM, (get_duty_cycle(MPPT_2_PWM) + mppt_calculate(&mppt_two)));
                }
                else if(mppt_two.suspended == true) {
                    // see if PV voltage is higher suspended voltage reading plus hysteresis
                    float latest_mppt_2 = get_force_mppt_v(mppt_two.mppt_base);
                    if(latest_mppt_2 - mppt_two.suspended_v >= 2.0) {
                        ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER3, ADC_TRIGGER_CPU1_TINT2, ADC_CH_ADCIN7, 15);
                    }
                }
                set_mppt_active(false);
            */
        }
        else if ((get_pid_active() == false) && (get_mppt_active() == false)) {
            // go to low-power mode until a timer interrupt wakes up CPU
            IDLE;
        }
#endif
#if defined(TEST_OUTPUT_BUCKS)
        if(get_pid_active() == true) {
            change_pwm_duty_cycle(BUCK3V3_BASE, PID_calculate(&five_volt_buck_pid, get_buck_v(BUCK3V3_BASE)));
            change_pwm_duty_cycle(BUCK5V0_BASE, PID_calculate(&three_volt_buck_pid, get_buck_v(BUCK5V0_BASE)));
            set_pid_active(false);
        }
        else if (get_pid_active() == false) {
            // go to low-power mode until a timer interrupt wakes up CPU
            IDLE;
        }
#endif
#if defined(TEST_MPPT_BUCKS)
        if(get_mppt_active() == true) {
            /** Update values **/
            mppt_update_values(&mppt_one);
            mppt_update_values(&mppt_two);

            /** Change duty cycles **/
            change_pwm_duty_cycle(MPPT_1_PWM, (get_duty_cycle(MPPT_1_PWM) + mppt_calculate(&mppt_one)));
            change_pwm_duty_cycle(MPPT_2_PWM, (get_duty_cycle(MPPT_2_PWM) + mppt_calculate(&mppt_two)));

            set_mppt_active(false);
        }
        else if (get_mppt_active() == false) {
            // go to low-power mode until a timer interrupt wakes up CPU
            IDLE;
        }
#endif
    }
}
