#ifndef TEMPERATURE_HUMIDITY_SENSOR_H
#define TEMPERATURE_HUMIDITY_SENSOR_H

/************************************************
 *  Includes
 ***********************************************/
#include "HomeSpan.h"
#include "DHT.h"

/* Local files */
#include "devices/deviceInfo.h"

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

    /** @brief DHT Temperature Object */
    DHT tempSensor{DHT_PIN, DHT_TYPE};

public:
    /** @brief Constructor */
    HS_TempSensor() : Service::TemperatureSensor() {
        /* Initialize the DHT Sensor */
        tempSensor.begin();

        /* Get the initial temperature */
        temp = new Characteristic::CurrentTemperature(tempSensor.readTemperature());

        /* Set default values for temperature and humidity ranges */
        setTempRange(DHT_TEMPERATURE_DEFAULT_MIN_VAL, DHT_TEMPERATURE_DEFAULT_MAX_VAL);
    }

    /** @brief Loop function override */
    void loop() override {

        /* if it has been a while since last update */
        if (temp->timeVal() > TEMPERATURE_SENSOR_POLLING_TIME) {

            /* Set the temperature & humidity */
            temp->setVal(tempSensor.readTemperature());
        }
    }

    /**
     * @brief Set Temp Range method
     * @details
     *  Set the temperature range
     *
     * @param min   min temperature threshold
     * @param max   max temperature threshold
     */
    void setTempRange(float min, float max) {
        float minTempVal = 0;
        float maxTempVal = 0;

        /* Check for min and max range */
        if (min < DHT_TEMPERATURE_DEFAULT_MIN_VAL) {
            minTempVal = DHT_TEMPERATURE_DEFAULT_MIN_VAL;
        } else {
            minTempVal = min;
        }

        if (max > DHT_TEMPERATURE_DEFAULT_MAX_VAL) {
            maxTempVal = DHT_TEMPERATURE_DEFAULT_MAX_VAL;
        } else {
            maxTempVal = max;
        }

        temp->setRange(minTempVal, maxTempVal);
    }
};

#endif /* TEMPERATURE_HUMIDITY_SENSOR_H */
