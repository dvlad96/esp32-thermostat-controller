/************************************************
 *  Includes
 ***********************************************/
#include <Arduino.h>
#include "HomeSpan.h"

/* Local files */
#include "homeKitAccessories/thermostat.h"
#include "homeKitAccessories/airConditioner.h"
#include "devices/daikin.h"

/* Private files */
#include "private/wifiCredentials.h"

/************************************************
 *  Defines / Macros
 ***********************************************/

/** @brief Daikin AC IP Address */
#define DAIKIN_IP_ADDRESS       ("192.168.1.132")

/** @brief Daikin AC Port ID */
#define DAIKIN_PORT_ID          (DEFAULT_DAIKIN_PORT)

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
        new HS_Thermostat((char *)DAIKIN_IP_ADDRESS, DAIKIN_PORT_ID);

    /* 3. Living Room AC */
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();
            new Characteristic::Name("Living Room AC");
        new HS_AirConditioner((char *)DAIKIN_IP_ADDRESS, DAIKIN_PORT_ID);
}

void loop() {
    homeSpan.poll();
}
