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
    return(tempEvent.temperature);
}

float TempHumSensor::getCurrentHumidity(void) {
    tempSensor.getEvent(&humEvent, &tempEvent);
    return(humEvent.relative_humidity);
}

/************************************************
 *  Private Method implementation
 ***********************************************/
