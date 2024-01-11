/************************************************
 *  Includes
 ***********************************************/
#include <Arduino.h>
#include "HomeSpan.h"

/* Local files */
#include "homeKitAccessories/thermostat.h"
#include "homeKitAccessories/tempHumSensor.h"
#include "homeKitAccessories/relaySwitch.h"
#include "devices/deviceInfo.h"

/* Private files */
#include "private/wifiCredentials.h"

/************************************************
 *  Defines / Macros
 ***********************************************/
/** @brief Maximum number of log messages to save.
 *  As the log fills with messages, older ones are replaced by newer ones. */
#define MAX_NB_LOG_MESSAGES_TO_SAVE             (25U)

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Public function implementation
 ***********************************************/
void setup() {
#if defined(WIFI_SSID) && defined(WIFI_PASSWORD)
    /* Connect to the Internet */
    homeSpan.setWifiCredentials(WIFI_SSID, WIFI_PASSWORD);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

    /* Enable debug options */
    Serial.begin(115200);
    homeSpan.enableWebLog(MAX_NB_LOG_MESSAGES_TO_SAVE, "pool.ntp.org", "UTC+4", "myLog");
    homeSpan.enableOTA();

    /* Create the pairing code */
    homeSpan.setPairingCode("00011000");

    /* Create a Bridge to account for multiple devices */
    homeSpan.begin(Category::Bridges, "ESP32 - Bridge");

    /* 1. Bridge */
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();

    /* 2. Living Room Temperature & Humidity Sensor */
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(TEMP_HUM_SENSOR_NAME);
            new Characteristic::Manufacturer(TEMP_HUM_SENSOR_MANUFACTURER);
            new Characteristic::Model(TEMP_HUM_SENSOR_MODEL);
            new Characteristic::SerialNumber(TEMP_HUM_SENSOR_SERIAL_NUM);
            new Characteristic::FirmwareRevision(TEMP_HUM_SENSOR_FIRMWARE);
            new Characteristic::Identify();
        new HS_TempSensor();

    /* 3. Living Room Thermostat */
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name(THERMOSTAT_NAME);
            new Characteristic::Manufacturer(THERMOSTAT_MANUFACTURER);
            new Characteristic::Model(THERMOSTAT_MODEL);
            new Characteristic::SerialNumber(THERMOSTAT_SERIAL_NUM);
            new Characteristic::FirmwareRevision(THERMOSTAT_FIRMWARE);
            new Characteristic::Identify();
        new HS_Thermostat();

    /* 4. Heating relay - defined as switch */
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name("Heating Relay");
            new Characteristic::Identify();
        new HS_RelaySwitch();
}

void loop() {
    homeSpan.poll();
}
