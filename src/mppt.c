/*
 * mppt.c
 *
 *  Created on: Jan 5, 2020
 *      Author: jack
 */

#include "driverlib.h"

#include "mppt.h"
#include "src_adc.h"
#include "src_epwm.h"
#include <stdint.h>

/**************************************************
 * mppt_init
 *
 * @brief Initializes an instance of the MPPT struct
 *
 * @param mppt  Instance of the MPPT structure
 *
 * @param mppt_base EPWM base that this MPPT strcuture is associated with
 *
 * @param delta_d How much to vary the duty cycle by to find the maximum power point
 *
 * @param delta_max How much to vary the duty cycle by if a maximum voltage or current reading is made
 *
 **************************************************/
void mppt_init(MPPT_t * mppt, uint32_t mppt_base, float delta_d, float delta_max) {
    mppt->mppt_base = mppt_base;
    mppt->delta_d = delta_d;
    mppt->delta_max = delta_max;

    mppt->v_result = 0;
    mppt->v_old = 0;
    mppt->i_result = 0;
    mppt->i_old = 0;
//    mppt->power = 0;
    mppt->power_old = 0;
    mppt->delta_v = 0;
    mppt->delta_i = 0;
    mppt->delta_p = 0;
}


/**************************************************
 * mppt_update_values
 *
 * @brief Samples the associated MPPT converter's voltage and current
 *      and updates the corresponding values in the MPPT structure
 *
 * @param mppt Instance of the MPPT structure
 *
 *************************************************/
void mppt_update_values(MPPT_t * mppt) {
    // get updated values from ADC conversions
    mppt->v_result = get_mppt_v(mppt->mppt_base);
    mppt->i_result = get_mppt_i(mppt->mppt_base);
    mppt->power = mppt->v_result * mppt->i_result;

    // calculate delta values
    mppt->delta_v = mppt->v_result - mppt->v_old;
    mppt->delta_i = mppt->i_result - mppt->i_old;
    mppt->delta_p = mppt->power - mppt->power_old;

    // make current values old values
    mppt->v_old = mppt->v_result;
    mppt->i_old = mppt->i_result;
    mppt->power_old = mppt->power;
}

/*************************************************
 * mppt_calculate
 *
 * @brief Implements an MPPT algorithm with CC/CV battery charging
 *
 * @details Combines the MPPT algorithm with the CC/CV battery charging algorithm
 *  to ensure than the battery voltage and current limits are not gone over. If the
 *  PV panel voltage is below the battery voltage, the converter will be disabled
 *
 *  @return How much to change the duty cycle by
 *
 *************************************************/
float mppt_calculate(MPPT_t * mppt) {
    /** CC/CV */

    /* current is too high */
//    if(mppt->i_result > I_BATTERY_LIMIT) {
//        return -(mppt->delta_max * (mppt->i_result - I_BATTERY_LIMIT));
//    }
//
//    /* voltage is too high */
//    else if(mppt->v_result > get_battery_v()) {
//        return -(mppt->delta_max * (mppt->v_result - V_BATTERY_LIMIT));
//    }

    /* voltage is too low */
//    else if(mppt->v_result < V_BATTERY_MIN_LIMIT) {
//        /* turn off converter since PV voltage is too low */
//        return -(get_duty_cycle(mppt->mppt_base));  // returns value to get duty cycle to zero
//    }

    /** MPPT */
    float ret;
    if((mppt->delta_p * mppt->delta_v) > 0) {
        ret = -mppt->delta_d;
        GPIO_writePin(25, 0);
    }
    else {
        ret = mppt->delta_d;
        GPIO_writePin(25, 1);
    }
    return ret;
}
