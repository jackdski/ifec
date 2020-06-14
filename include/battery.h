/*
 * battery.h
 *
 *  Created on: Jun 13, 2020
 *      Author: jack
 */

#ifndef INCLUDE_BATTERY_H_
#define INCLUDE_BATTERY_H_

typedef enum {
    Supply,
    Charge
} eBatteryState;

typedef enum {
    PV1,
    PV2,
    Not_Charging
} eChargeSource;

typedef enum {
    Charging_Inactive,
    Continuous_Current,
    Continuous_Voltage,
    Battery_Full
} eBatteryChargeType;

typedef struct {
    eBatteryChargeType  cc_cv;
    eChargeSource       source;
} Charger_t;

typedef struct {
    float               voltage;
    float               current;
    eBatteryState       state;
    Charger_t           charger;
} Battery_t;

void init_battery(Battery_t * battery);

void update_battery(Battery_t * battery);

eBatteryChargeType determine_cc_cv(Battery_t battery);

void determine_battery_state(Battery_t * battery);

#endif /* INCLUDE_BATTERY_H_ */
