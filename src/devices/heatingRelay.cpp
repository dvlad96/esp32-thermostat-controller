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

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Static function implementation
 ***********************************************/

/************************************************
 *  Public Method Implementation
 ***********************************************/
t_httpErrorCodes heatingRelay::sendRelayCommand(const t_esp01sRelayState command) {

    t_httpErrorCodes commandError;

    /* Set the relay state */
    if (command == E_ESP01S_RELAY_OPEN) {
        commandError = sendEsp01sRelayCommand(RELAY_OPEN_COMMAND);
    } else if (command == E_ESP01S_RELAY_CLOSE) {
        commandError = sendEsp01sRelayCommand(RELAY_CLOSE_COMMAND);
    } else {
        /* Can't reach here */
    }

    return (commandError);
}

/************************************************
 *  Private Method implementation
 ***********************************************/
