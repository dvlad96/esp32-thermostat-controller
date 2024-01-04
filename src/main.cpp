/************************************************
 *  Includes
 ***********************************************/
#include <Arduino.h>
#include "HomeSpan.h"

/* Local files */
#include "homeKitAccessories/thermostat.h"
#include "homeKitAccessories/airConditioner.h"
#include "homeKitAccessories/tempHumSensor.h"
#include "devices/daikin.h"
#include "devices/heatingRelay.h"
#include "devices/deviceInfo.h"

/* Private files */
#include "private/wifiCredentials.h"

/************************************************
 *  Defines / Macros
 ***********************************************/

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Public function implementation
 ***********************************************/
void setup() {

    /* Used for debug */
    Serial.begin(115200);

#if defined(WIFI_SSID) && defined(WIFI_PASSWORD)
    /* Connect to the Internet */
    homeSpan.setWifiCredentials(WIFI_SSID, WIFI_PASSWORD);
#endif

    /* Create the pairing code */
    homeSpan.setPairingCode("00011000");

    /* Create a Bridge to account for multiple devices (accessories) */
    homeSpan.begin(Category::Bridges, "ESP32 - Bridge");

    /* 1. Bridge */
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();

    /* 2. Living Room Thermostat */
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(THERMOSTAT_NAME);
            new Characteristic::Manufacturer(THERMOSTAT_MANUFACTURER);
            new Characteristic::Model(THERMOSTAT_MODEL);
            new Characteristic::SerialNumber(THERMOSTAT_SERIAL_NUM);
            new Characteristic::FirmwareRevision(THERMOSTAT_FIRMWARE);
            new Characteristic::Identify();
        new HS_Thermostat();

    /* 3. Living Room AC */
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(DAIKIN_AC_NAME);
            new Characteristic::Manufacturer(DAIKIN_AC_MANUFACTURER);
            new Characteristic::Model(DAIKIN_AC_MODEL);
            new Characteristic::SerialNumber(DAIKIN_AC_SERIAL_NUM);
            new Characteristic::FirmwareRevision(DAIKIN_AC_FIRMWARE);
            new Characteristic::Identify();
        new HS_AirConditioner();

    /* 4. Living Room Temperature & Humidity Sensor */
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(DHT_SENSOR_NAME);
            new Characteristic::Manufacturer(DHT_SENSOR_MANUFACTURER);
            new Characteristic::Model(DHT_SENSOR_MODEL);
            new Characteristic::SerialNumber(DHT_SENSOR_SERIAL_NUM);
            new Characteristic::FirmwareRevision(DHT_SENSOR_FIRMWARE);
            new Characteristic::Identify();
        new HS_TempSensor();
}

void loop() {
    homeSpan.poll();
}
