#include <Arduino.h>
#include "HomeSpan.h"

/* Local files */
#include "thermostat.h"
#include "devices/daikin.h"

#define WIFI_SSID       ("DIGI-yYs4")
#define WIFI_PASSWORD   ("7Az3A6D2Zz")

#define DAIKIN_IP_ADDRESS   ("192.168.1.132")
#define DAIKIN_PORT_ID      (80U)

void setup() {

    Serial.begin(115200);

#if defined(WIFI_SSID) && defined(WIFI_PASSWORD)
    /* Connect to the Internet */
    homeSpan.setWifiCredentials(WIFI_SSID, WIFI_PASSWORD);
#endif

    /* Create a 6 digit pairing code */
    homeSpan.setPairingCode("00011000");

    /* Create the Home Span accessory */
    homeSpan.begin(Category::Thermostats, "HomeSpan Thermostat");

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();
        new HS_Thermostat((char *)DAIKIN_IP_ADDRESS, DAIKIN_PORT_ID);
}

void loop() {
    homeSpan.poll();
}
