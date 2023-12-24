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

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Class definition
 ***********************************************/
struct HS_TempHumSensor : Service::TemperatureSensor, DHT {
  public:
    HS_TempHumSensor(const int dhtPin, const int dhtType, const int pollingTime) : Service::TemperatureSensor(), DHT(dhtPin, dhtType) {
      /* Initialize the DHT Sensor */
      begin();
      this->pollingTime = pollingTime;

      /* Get the initial temperature */
      temp = new Characteristic::CurrentTemperature(readTemperature());

      /* Set temperature range */
      temp->setRange(15,30);
    } /* end constructor */

    void loop() override {
      /* if it has been a while since last update */
      if(temp->timeVal() > pollingTime) {
        /* Get the temperature */
        temperature = readTemperature();

        /* Set the temperature */
        temp->setVal(temperature);

        /* Debug */
        Serial.printf("New readout temperature = %f.2", temperature);
      }
    } /* loop */

  private:
    SpanCharacteristic *temp;
    float temperature;
    uint32_t pollingTime;
};

#endif /* TEMPERATURE_HUMIDITY_SENSOR_H */
