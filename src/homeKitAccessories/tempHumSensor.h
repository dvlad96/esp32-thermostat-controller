#ifndef TEMPERATURE_HUMIDITY_SENSOR_H
#define TEMPERATURE_HUMIDITY_SENSOR_H

/************************************************
 *  Includes
 ***********************************************/
#include "HomeSpan.h"
#include "DHT.h"

/* Local files */

/************************************************
 *  Defines / Macros
 ***********************************************/
#define DHT_TEMPERATURE_DEFAULT_MIN_VAL         (15U)
#define DHT_TEMPERATURE_DEFAULT_MAX_VAL         (30U)

#define DHT_HUMIDITY_DEFAULT_MIN_RANGE          (0U)
#define DHT_HUMIDITY_DEFAULT_MAX_RANGE          (100U)

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Class definition
 ***********************************************/
struct HS_TempHumSensor : Service::TemperatureSensor, Service::HumiditySensor, DHT {
  public:
    HS_TempHumSensor(const int dhtPin, const int dhtType, const int pollingTime) : Service::TemperatureSensor(), Service::HumiditySensor(), DHT(dhtPin, dhtType) {
      /* Initialize the DHT Sensor */
      begin();
      this->pollingTime = pollingTime;

      /* Get the initial temperature */
      temp = new Characteristic::CurrentTemperature(getTemperature());
      humidity = new Characteristic::CurrentRelativeHumidity(getHumidity());

      /* Set default values for temperature and humidity ranges */
      setTempRange(DHT_TEMPERATURE_DEFAULT_MIN_VAL, DHT_TEMPERATURE_DEFAULT_MAX_VAL);
      setHumidityRange(DHT_HUMIDITY_DEFAULT_MIN_RANGE, DHT_HUMIDITY_DEFAULT_MAX_RANGE);
    } /* end constructor */

    void loop() override {

        /* if it has been a while since last update */
        if (temp->timeVal() > pollingTime) {

            /* Set the temperature & humidity */
            temp->setVal(getTemperature());
            humidity->setVal(getHumidity());

            /* Debug */
            Serial.printf("New readout temperature = %.2f \n", temp->getNewVal());
            Serial.printf("New readout humidity = %.2f \n", humidity->getNewVal());
        }
    }

    void setTempRange(float min, float max);
    void setHumidityRange(uint8_t min, uint8_t max);
    float getTemperature(void);
    float getHumidity(void);

  private:
    SpanCharacteristic *temp;
    SpanCharacteristic *humidity;
    uint32_t pollingTime;
};

#endif /* TEMPERATURE_HUMIDITY_SENSOR_H */
