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

/* Private files */
#include "private/wifiCredentials.h"

/************************************************
 *  Defines / Macros
 ***********************************************/

/** @brief Daikin AC IP Address */
#define DAIKIN_IP_ADDRESS               ("192.168.1.132")

/** @brief Daikin AC Port ID */
#define DAIKIN_PORT_ID                  (DEFAULT_DAIKIN_PORT)

#define LIVING_ROOM_DHT_PIN             (5U)
#define LIVING_ROOM_DHT_TYPE            (22U)
#define LIVING_ROOM_DHT_POLLING_TIME    (5000U)

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
    homeSpan.begin(Category::Bridges,"ESP32 - Bridge");

    /* 1. Bridge */
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();

    /* 2. Living Room Thermostat */
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();
            new Characteristic::Name("Living Room Thermostat");
        new HS_Thermostat((char *)DAIKIN_IP_ADDRESS, DAIKIN_PORT_ID,
                          LIVING_ROOM_DHT_PIN, LIVING_ROOM_DHT_TYPE, LIVING_ROOM_DHT_POLLING_TIME);

    /* 3. Living Room AC */
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();
            new Characteristic::Name("Living Room AC");
        new HS_AirConditioner((char *)DAIKIN_IP_ADDRESS, DAIKIN_PORT_ID);

    /* 4. Living Room Temperature & Humidity Sensor */
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();
            new Characteristic::Name("Living Room Temperature");
        new HS_TempHumSensor(LIVING_ROOM_DHT_PIN, LIVING_ROOM_DHT_TYPE, LIVING_ROOM_DHT_POLLING_TIME);

    relayCommunicationSetup();
}

void loop() {
    homeSpan.poll();
}
