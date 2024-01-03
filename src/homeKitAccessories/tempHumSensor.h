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
    SpanCharacteristic *temp;
    DHT tempSensor{DHT_PIN, DHT_TYPE};
    uint32_t pollingTime;

  public:
    HS_TempSensor() : Service::TemperatureSensor() {
        /* Initialize the DHT Sensor */
        tempSensor.begin();

        pollingTime = DHT_POLLING_TIME;

        /* Get the initial temperature */
        temp = new Characteristic::CurrentTemperature(tempSensor.readTemperature());

        /* Set default values for temperature and humidity ranges */
        setTempRange(DHT_TEMPERATURE_DEFAULT_MIN_VAL, DHT_TEMPERATURE_DEFAULT_MAX_VAL);
    } /* end constructor */

    void loop() override {

        /* if it has been a while since last update */
        if (temp->timeVal() > pollingTime) {

            /* Set the temperature & humidity */
            temp->setVal(tempSensor.readTemperature());
        }
    }

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