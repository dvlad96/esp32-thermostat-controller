#ifndef RELAY_SWITCH_H
#define RELAY_SWITCH_H

/************************************************
 *  Includes
 ***********************************************/
#include "HomeSpan.h"

/* Local files */
#include "devices/deviceInfo.h"
#include "devices/esp01sRelay.h"

/************************************************
 *  Defines / Macros
 ***********************************************/
/** @brief Update homekit device status time in ms */
#define GET_STATUS_REFRESH_TIME_IN_MS           (30 * 1000)

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Class definition
 ***********************************************/
struct HS_RelaySwitch : Service::Switch {
    /* Characteristic */
    SpanCharacteristic *power;

    /* ESP-01S Relay */
    Esp01sRelay relay {THERMOSTAT_RELAY_IP_ADDRESS, THERMOSTAT_RELAY_PORT_ID};

    HS_RelaySwitch() : Service::Switch()  {
        t_esp01sRelayState state;
        power = new Characteristic::On();

        /* Get the current relay state */
        (void)relay.getEsp01sRelayState(&state);
        power->setVal<bool>(
            state == E_ESP01S_RELAY_OPEN ? false : true
        );
    }

    boolean update()
    {
        return (true);
    }

    void loop() override {
        /* Check the status of the relay */
        if (power->timeVal() > GET_STATUS_REFRESH_TIME_IN_MS) {
            t_esp01sRelayState state;
            (void)relay.getEsp01sRelayState(&state);
            power->setVal<bool>(
                state == E_ESP01S_RELAY_OPEN ? false : true
            );
            WEBLOG("Relay state = %s", state == E_ESP01S_RELAY_OPEN ? "OPEN" : "CLOSE");
        }
    }
};

#endif /* RELAY_SWITCH_H */