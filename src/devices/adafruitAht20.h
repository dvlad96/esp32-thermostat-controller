#ifndef ADAFRUIT_AHT20_H
#define ADAFRUIT_AHT20_H

/************************************************
 *  Includes
 ***********************************************/
#include <Adafruit_AHTX0.h>

/************************************************
 *  Defines / Macros
 ***********************************************/
/** @brief Initial temperature value */
#define TEMPERATURE_INITIAL_VALUE           (22U)

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Class definition
 ***********************************************/
class TempHumSensor {
private:
    /** @brief Adafruit Object */
    Adafruit_AHTX0 tempSensor;

    /** @brief Temperature event type */
    sensors_event_t tempEvent;

    /** @brief Humidity event type */
    sensors_event_t humEvent;

public:
    /** @brief Constructor */
    TempHumSensor() {};

    /**
     * @brief Initialize sensor public method
     * @details
     *  This method is used to initialize the Adafruit ADH20 sensor
     *
     * @return true     If the sensor is initialized successfully
     * @return false    Most likely a wiring problem
     */
    bool initializeSensor(void);

    /**
     * @brief Get the Current Temperature public method
     * @details
     *  This method is used to get the current readout temperature
     *
     * @return temperature
     */
    float getCurrentTemperature(void);

    /**
     * @brief Get the Current Humidity public method
     * @details
     *  This method is used to get the current readout humidity
     *
     * @return humidity
     */
    float getCurrentHumidity(void);
};

#endif /* ADAFRUIT_AHT20_H */
