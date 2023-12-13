#include <Arduino.h>
#include "HomeSpan.h"

/* Local files */
#include "thermostat.h"


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
        new HS_Thermostat();
}

void loop() {
    homeSpan.poll();
}
