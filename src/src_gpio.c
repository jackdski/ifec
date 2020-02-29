/*
 * src_gpio.c
 *
 *  Created on: Jan 3, 2020
 *      Author: jack
 */

#include "src_gpio.h"
#include "driverlib.h"
#include "device.h"

/**********************************************************
 *                      I N I T S
 **********************************************************/

void init_led5(void) {
    GPIO_setPadConfig(DEVICE_GPIO_PIN_LED1, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LED1, GPIO_DIR_MODE_OUT);
    GPIO_writePin(DEVICE_GPIO_PIN_LED1, 0);
}

void toggle_led(void) {
    GPIO_togglePin(DEVICE_GPIO_PIN_LED1);
}
