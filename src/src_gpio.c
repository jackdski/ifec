/*
 * src_gpio.c
 *
 *  Created on: Jan 3, 2020
 *      Author: jack
 */

#include "src_gpio.h"
#include "driverlib.h"
#include "device.h"

void init_led5(void) {
    GPIO_setPadConfig(DEVICE_GPIO_PIN_LED1, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_LED1, GPIO_DIR_MODE_OUT);
    GPIO_writePin(DEVICE_GPIO_PIN_LED1, 0);

    GPIO_setPadConfig(25, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(25, GPIO_DIR_MODE_OUT);
    GPIO_writePin(25, 0);
}
