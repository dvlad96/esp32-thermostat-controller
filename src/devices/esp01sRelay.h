#ifndef ESP_01_S_RELAY_H
#define ESP_01_S_RELAY_H

/************************************************
 *  Includes
 ***********************************************/
#include "Arduino.h"
#include <esp_now.h>

/* Local files */
#include "private/espNowInfo.h"
#include "errorCodes.h"

/************************************************
 *  Defines / Macros
 ***********************************************/
#define RELAY_CLOSE_COMMAND     ("/relay_command?val=1")

#define RELAY_OPEN_COMMAND      ("/relay_command?val=0")

/************************************************
 *  Typedef definition
 ***********************************************/

/** @brief ESP-01S Relay State */
typedef enum {
    E_ESP01S_RELAY_OPEN  = 0U,                    /**< ESP-01S Relay OFF */
    E_ESP01S_RELAY_CLOSE = 1U                     /**< ESP-01S Relay ON */
} t_esp01sRelayState;

/************************************************
 *  Class definition
 ***********************************************/
/**
 * @brief ESP-01S class definition.
 * @details This class is used to define a generic ESP-01S Relay
 */
class esp01sRelay {
private:
    /** @brief ESP-01S relay state */
    t_esp01sRelayState internalRelayState;

    /** @brief URL Start */
    String httpCommand;

    /** @brief HTTP Client Object */
    HTTPClient http;

public:
    /** @brief Constructor */
    esp01sRelay(const String relayIpAddress, const uint8_t portId) {
        relayState = E_ESP01S_RELAY_OPEN;
        httpCommand = "http://" + relayIpAddress + ":" + String(portId);
    }

    /**
     * @brief Send commands to a generic ESP-01S device
     * @details
     *  This function is used to send commands to the ESP-01S Relay using HTTP GET commands
     *
     * @param command       HTTP GET command to the relay
     *
     * @return t_espNowErrorCodes
     */
    t_httpErrorCodes sendEsp01sRelayCommand(const String command);

    /**
     * @brief Get the Esp01s Relay State
     * @details
     *  This function is used to retrieve the status of the ESP-01S Relay using HTTP GET commands
     * @return t_esp01sRelayState
     */
    t_esp01sRelayState getEsp01sRelayState(void);
};

#endif /* ESP_01_S_RELAY_H */
