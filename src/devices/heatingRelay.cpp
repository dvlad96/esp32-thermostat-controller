/************************************************
 *  Includes
 ***********************************************/
#include <Arduino.h>

/* Local files */
#include "heatingRelay.h"

/************************************************
 *  Defines / Macros
 ***********************************************/
#define RELAY_COMMAND_SUCCESS       (true)
#define RELAY_COMMAND_FAILURE       (false)

#define TEST_LED                    (18U)

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Static function implementation
 ***********************************************/

/************************************************
 *  Public Method Implementation
 ***********************************************/
void relayCommunicationSetup() {

    /* For the moment test with a LED */
    pinMode(TEST_LED, OUTPUT);
    digitalWrite(TEST_LED, LOW);
}

bool relayOnOff(bool on) {

    if (on == HEATING_ON) {
        /* Send ON to the Heating Plant */
        digitalWrite(TEST_LED, HIGH);
    } else {
        /* Send OFF to the Heating Plant */
        digitalWrite(TEST_LED, LOW);
    }

    return (RELAY_COMMAND_SUCCESS);
}

/************************************************
 *  Private Method implementation
 ***********************************************/

