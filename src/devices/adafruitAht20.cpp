/************************************************
 *  Includes
 ***********************************************/
#include "adafruitAht20.h"

/************************************************
 *  Defines / Macros
 ***********************************************/

/************************************************
 *  Typedef definition
 ***********************************************/
#define TEMP_CALIBRATION_VALUE              (1.5f)
/************************************************
 *  Static function implementation
 ***********************************************/

/************************************************
 *  Public Method Implementation
 ***********************************************/
bool TempHumSensor::initializeSensor(void) {
    /* Initialize the sensor */
    return (tempSensor.begin());
}

float TempHumSensor::getCurrentTemperature(void) {
    tempSensor.getEvent(&humEvent, &tempEvent);
    return(tempEvent.temperature - TEMP_CALIBRATION_VALUE);
}

float TempHumSensor::getCurrentHumidity(void) {
    tempSensor.getEvent(&humEvent, &tempEvent);
    return(humEvent.relative_humidity);
}

/************************************************
 *  Private Method implementation
 ***********************************************/
