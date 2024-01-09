#ifndef TEMPERATURE_HUMIDITY_SENSOR_H
#define TEMPERATURE_HUMIDITY_SENSOR_H

/************************************************
 *  Includes
 ***********************************************/
#include "HomeSpan.h"

/* Local files */
#include "devices/deviceInfo.h"
#include "devices/adafruitAht20.h"

/************************************************
 *  Defines / Macros
 ***********************************************/

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Class definition
 ***********************************************/
struct HS_TempSensor : Service::TemperatureSensor {
private:
    /** @brief Temperature Characteristic */
    SpanCharacteristic * temp;

    /** @brief TempHumSensor Object */
    TempHumSensor tempSensor;

public:
    /** @brief Constructor */
    HS_TempSensor() : Service::TemperatureSensor() {
        /* Initialize the Temperature & Humidity Sensor */
        (void)tempSensor.initializeSensor();

        /* Get the initial temperature */
        temp = new Characteristic::CurrentTemperature(TEMPERATURE_INITIAL_VALUE);

        /* Set default values for temperature and humidity ranges */
        temp->setRange(TEMPERATURE_DEFAULT_MIN_VAL, TEMPERATURE_DEFAULT_MAX_VAL);
    }

    /** @brief Loop function override */
    void loop() override {
        /* If it has been a while since last update */
        if (temp->timeVal() > TEMPERATURE_SENSOR_POLLING_TIME) {
            /* Set the temperature */
            temp->setVal(tempSensor.getCurrentTemperature());
        }
    }
};

#endif /* TEMPERATURE_HUMIDITY_SENSOR_H */
