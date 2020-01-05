/*
 * pid.h
 *
 *  Created on: Jan 4, 2020
 *      Author: jack
 */

#ifndef INCLUDE_PID_H_
#define INCLUDE_PID_H_

#include <stdint.h>

typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float error;
    float error_prior;
    float integral;
    float integral_prior;
    float derivative;
    float ref;
    uint16_t iteration_time;    // in ms
}PID_t;

void PID_init(PID_t * pid, float Kp, float Ki, float Kd, float ref, float iteration_time);
void PID_calculate_error(PID_t * pid, float actual_value);
void PID_calculate_integral(PID_t * pid);
void PID_calculate_derivative(PID_t * pid);
float PID_calculate(PID_t * pid, float actual_value);

#endif /* INCLUDE_PID_H_ */
