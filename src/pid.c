/*
 * pid.c
 *
 *  Created on: Jan 4, 2020
 *      Author: jack
 */

#include "pid.h"

void PID_init(PID_t * pid, float Kp, float Ki, float Kd, float ref, float iteration_time) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->ref = ref;
    pid->iteration_time = iteration_time;

    pid->derivative = 0;
    pid->error = 0;
    pid->error_prior = 0;
    pid->integral = 0;
    pid->integral_prior = 0;
}

void PID_calculate_error(PID_t * pid, float actual_value) {
    pid->error = pid->ref - actual_value;
}

void PID_calculate_integral(PID_t * pid) {
    pid->integral = pid->integral_prior + (pid->error * pid->iteration_time);
    pid->integral_prior = pid->integral;
}

void PID_calculate_derivative(PID_t * pid) {
    pid->derivative = (pid->error - pid->error_prior) / pid->iteration_time;
}

float PID_calculate(PID_t * pid, float actual_value) {
    PID_calculate_error(pid, actual_value);
    PID_calculate_integral(pid);
    PID_calculate_derivative(pid);
    pid->error_prior = pid->error;

    float output;
    output = ((pid->Kp * pid->error) + (pid->Ki * pid->integral) + (pid->Kd * pid->derivative));
    return output;
}
