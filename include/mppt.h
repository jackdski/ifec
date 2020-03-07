/*
 * mppt.h
 *
 *  Created on: Mar 6, 2020
 *      Author: jack
 */

#ifndef INCLUDE_MPPT_H_
#define INCLUDE_MPPT_H_

#include <stdint.h>
#include <stdbool.h>

#define V_BATTERY_LIMIT     7.95    // [V]
#define V_BATTERY_MIN_LIMIT 6.0     // [V]
#define I_BATTERY_LIMIT     3000.0  // [mA]

#define PV_HYSTERISIS       3.00    // [V]

typedef struct {
    float v_result;     // [V]
    float v_old;        // [V]
    float i_result;     // [mA]
    float i_old;        // [mA]
    float power;        // [mW]
    float power_old;    // [mW]
    float delta_v;      // [V]
    float delta_i;      // [mA]
    float delta_p;      // [mW]
    float delta_d;      // change in duty cycle
    float delta_max;    // change in duty cycle to be used with CC/CV
    uint32_t mppt_base; // MPPT instance identifier
//    bool suspended;     // for when PV voltage is too low
    float suspended_v;  // [V] when PV was suspended
}MPPT_t;


void mppt_init(MPPT_t * mppt, uint32_t mppt_base, float delta_d, float delta_max);
void mppt_update_values(MPPT_t * mppt);
float mppt_calculate(MPPT_t * mppt);


#endif /* INCLUDE_MPPT_H_ */
