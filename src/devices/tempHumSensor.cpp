/************************************************
 *  Includes
 ***********************************************/

/* Local files */
#include "homeKitAccessories/tempHumSensor.h"

/************************************************
 *  Defines / Macros
 ***********************************************/

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Static function implementation
 ***********************************************/

/************************************************
 *  Public Method Implementation
 ***********************************************/
void HS_TempHumSensor::setTempRange(float min, float max) {

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

void HS_TempHumSensor::setHumidityRange(uint8_t min, uint8_t max) {

    uint8_t minHumVal = 0;
    uint8_t maxHumVal = 0;

    /* Check for min and max range */
    if (min < DHT_HUMIDITY_DEFAULT_MIN_RANGE) {
        minHumVal = DHT_HUMIDITY_DEFAULT_MIN_RANGE;
    } else {
        minHumVal = min;
    }

    if (max > DHT_HUMIDITY_DEFAULT_MAX_RANGE) {
        maxHumVal = DHT_HUMIDITY_DEFAULT_MAX_RANGE;
    } else {
        maxHumVal = max;
    }

    humidity->setRange(minHumVal, maxHumVal);
}

float HS_TempHumSensor::getTemperature(void) {
    return (readTemperature());
}

float HS_TempHumSensor::getHumidity(void) {
    return (readHumidity());
}
