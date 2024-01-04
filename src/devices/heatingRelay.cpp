/************************************************
 *  Includes
 ***********************************************/
#include <Arduino.h>
#include <esp_now.h>

/* Local files */
#include "heatingRelay.h"

/************************************************
 *  Defines / Macros
 ***********************************************/
/** @brief ESP-Now command size in bytes */
#define HEATING_RELAY_COMMAND_SIZE          (1U)

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Static function implementation
 ***********************************************/

/************************************************
 *  Public Method Implementation
 ***********************************************/
t_espNowErrorCodes heatingRelay::sendRelayCommand(const uint8_t command) {

    t_espNowErrorCodes commandError = E_COMMAND_FAILURE;
    uint8_t espCommand[HEATING_RELAY_COMMAND_SIZE] = {
        command
    };

    /* Check if the ESP-Now Communication protocol is up */
    if (E_ESP_NOW_COMMUNICATION_UP == getEspNowRelayStatus()) {
        commandError = sendEspNowRelayCommand(espCommand, HEATING_RELAY_COMMAND_SIZE);
    }

    return (commandError);
}

/************************************************
 *  Private Method implementation
 ***********************************************/
