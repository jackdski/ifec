/*
 * mppt.c
 *
 *  Created on: Jan 5, 2020
 *      Author: jack
 */

#include "mppt.h"
#include "src_adc.h"
#include <stdint.h>

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

/**
 * Combines the MPPT algorithm with the CC/CV battery charging algorithm
 *  to ensure than the battery voltage and current limits are not gone over.
 */
float mppt_calculate(MPPT_t * mppt) {
    /** CC/CV */
    if(mppt->i_result > I_BATTERY_LIMIT) {
        return -(mppt->delta_max * (mppt->i_result - I_BATTERY_LIMIT));
    }
    else if(mppt->v_result > V_BATTERY_LIMIT) {
        return -(mppt->delta_max * (mppt->v_result - V_BATTERY_LIMIT));
    }

    /** MPPT */
    if((mppt->delta_p * mppt->delta_v) > 0) {
        return -mppt->delta_d;
    }
    else {
        return mppt->delta_d;
    }
}
