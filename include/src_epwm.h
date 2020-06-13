/*
 * src_epwm.h
 *
 *  Created on: Jan 3, 2020
 *      Author: jack
 */

#ifndef INCLUDE_SRC_EPWM_H_
#define INCLUDE_SRC_EPWM_H_

#include <stdint.h>


#define     SWITCHING_FREQUENCY     1000000     // [Hz]
#define     CLOCK_FREQUENCY         100000000   // [Hz]
#define     PERIOD                  (CLOCK_FREQUENCY / SWITCHING_FREQUENCY)


/***    I N I T S    ***/
void initEPWMGPIO(void);
void initEPWM1(void);
void initEPWM3(void);
void initEPWM(uint32_t epwm_base);

/***    D U T Y   C Y C L E    ***/
void change_pwm_duty_cycle(uint32_t epwm_base, float dc);
float get_duty_cycle(uint32_t epwm_base);

#endif /* INCLUDE_SRC_EPWM_H_ */
