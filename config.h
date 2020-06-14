/*
 * config.h
 *
 *  Created on: Jun 13, 2020
 *      Author: jack
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "driverlib.h"
#include "device.h"
#include <stdint.h>


/** TIMER CONSTANTS **/
#define TIMER_10US              10U         // 100kHz
#define TIMER_50US              50U         // 20kHz
#define TIMER_250US             250U        // 4kHz
#define TIMER_500US             500U        // 2kHz
#define TIMER_10MS              10000U      // 100Hz
#define TIMER_500MS             500000U     // 2Hz


/** BATTERY CONSTANTS **/
#define V_BATTERY_MAX_LIMIT     7.95f       // [V]
#define V_BATTERY_CHG_LIMIT     8.2f        // [V]
#define V_BATTERY_MIN_LIMIT     6.0f        // [V]
#define I_BATTERY_MAX_LIMIT     3.00f       // [A]
#define I_BATTERY_MIN_LIMIT     (0.05 * I_BATTERY_MAX_LIMIT)  // [A]

/** TIMER CONFIG **/
#define PID_TIMER               CPUTIMER1_BASE
#define PID_TIMER_INT           INT_TIMER1

#define MPPT_TIMER              CPUTIMER2_BASE
#define MPPT_TIMER_INT          INT_TIMER2


/** CONTROL LOOP CONSTANTS **/
#define PID_FREQUENCY           100000U // [Hz]
#define MPPT_FREQUENCY          500U    // [Hz]

#define FREQUENCY_TO_US(FREQ)   ((1 / FREQ) * (1000000))
#define PID_US                  FREQUENCY_TO_US(PID_FREQUENCY)

#define KP                      3.2f
#define KI                      2.1f
#define KD                      2.3f

#define MPPT_1_DELTA_DC         0.1f
#define MPPT_1_DELTA_DC_MAX     5.0f
#define MPPT_2_DELTA_DC         2.5f
#define MPPT_2_DELTA_DC_MAX     5.0f


/** VOLTAGE DIVIDER COMPONENTS **/
#define VOLTAGE_DIVDER(VS, R1, R2)  ((VS * R2) / (R1 + R2))
#define VOLTAGE_UNDIVIDER(VO, R1, R2)   ((VO / R2) * (R1 + R2))

#define V_PV_SENSE_R1           7500U   // [Ohms]
#define V_PV_SENSE_R2           1000U   // [Ohms]

#define V_BATT_SENSE_R1         100000U // [Ohms]
#define V_BATT_SENSE_R2         100000U // [Ohms]

#define BUCK_5V_OUTPUT_R1       5110U   // [Ohms]
#define BUCK_5V_OUTPUT_R2       5110U   // [Ohms]

#define BUCK_3V3_OUTPUT_R1      3090U   // [Ohms]
#define BUCK_3V3_OUTPUT_R2      7150U   // [Ohms]


/** REFERENCE VOLTAGES **/
#define V_BUCK_5V_REF           VOLTAGE_DIVDER(5.00, BUCK_5V_OUTPUT_R1, BUCK_5V_OUTPUT_R2)
#define V_BUCK_3V3_REF          VOLTAGE_DIVDER(3.30, BUCK_3V3_OUTPUT_R1, BUCK_3V3_OUTPUT_R2)


/** CURRENT SENSING COMPONENT **/
/* ACS70331EESATR-005U3 */
#define V_IOUT_Q                0.25f   // [V]
#define I_SENSE_SENS            400U    // [mV/A]
#define I_SENSE_MAX             5.0f    // [A]
#define I_SENSE_MIN             0.0f    // [A]
#define I_SENSED(V_IOUT)        (((V_IOUT - V_IOUT_Q) / I_SENSE_SENS))    // [A]


/** PINS & IDs **/

/*  Ex:
 *  #define USE                  GPIO_NUM    // HSEC_PIN - PERIPHERAL
 */

#define BUCK_5V_ID              0U
#define BUCK_5V_PWM             EPWM8_BASE
#define BUCK_5V_HI_PWM          14U // 61 - PWM8A
#define BUCK_5V_LI_PWM          15U // 63 - PWM8B

#define BUCK_3V3_ID             1U
#define BUCK_3V3_PWM            EPWM7_BASE
#define BUCK_3V3_HI_PWM         12 // 57 - PWM7A
#define BUCK_3V3_LI_PWM         13 // 59 - PWM7B

#define MPPT_ONE_ID             2U
#define MPPT_1_PWM              EPWM1_BASE
#define MPPT_1_HI_PWM           0U // 49 - PWM1A
#define MPPT_1_LI_PWM           1U // 51 - PWM1B

#define MPPT_TWO_ID             3U
#define MPPT_2_PWM              EPWM2_BASE
#define MPPT_2_HI_PWM           2U // 53 - PWM2A
#define MPPT_2_LI_PWM           3U // 55 - PWM2B

/**
 * TODO: Make sure the right ADC pins are used!
 *      Also look at src/src_adc.c
 **/

#define BATT_V_SENSE            40U         // 40 - ADC
#define BATT_I_SENSE            42U         // 42 - ADC

#define BUCK_5V_V_SENSE         44U     // 36 - C14/ADC
#define BUCK_3V3_V_SENSE        17U     // 34 - C4/ADC

#define PV1_V_SENSE             41U     // 12 - B0/ADC
#define PV1_I_SENSE             40U     // 14 - B1, A10, C10/ADC

#define PV2_V_SENSE             39      // 39 - ADC
#define PV2_I_SENSE             37      // 37 - ADC

#endif /* CONFIG_H_ */
