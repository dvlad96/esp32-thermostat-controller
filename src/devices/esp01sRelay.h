#ifndef ESP_01_S_RELAY_H
#define ESP_01_S_RELAY_H

/************************************************
 *  Includes
 ***********************************************/
#include "Arduino.h"
#include <HTTPClient.h>

/************************************************
 *  Defines / Macros
 ***********************************************/
/** @brief Relay HTTP GET close command */
#define RELAY_CLOSE_COMMAND     ("/relay_command?val=1")

/** @brief Relay HTTP GET open command */
#define RELAY_OPEN_COMMAND      ("/relay_command?val=0")

/************************************************
 *  Typedef definition
 ***********************************************/
/** @brief This enum represents all the possible HTTP Requests failures*/
typedef enum {
    E_REQUEST_SUCCESS = 0U,             /**< HTTP Request success */
    E_REQUEST_FAILURE = 1U              /**< HTTP Request failure */
} t_httpErrorCodes;

/** @brief ESP-01S Relay State */
typedef enum {
    E_ESP01S_RELAY_OPEN  = 0U,          /**< ESP-01S Relay Open */
    E_ESP01S_RELAY_CLOSE = 1U           /**< ESP-01S Relay Close */
} t_esp01sRelayState;

/************************************************
 *  Class definition
 ***********************************************/
/**
 * @brief ESP-01S class definition.
 * @details This class is used to define a generic ESP-01S Relay
 */
class Esp01sRelay {
private:
    /** @brief ESP-01S relay state */
    t_esp01sRelayState internalRelayState;

    /** @brief URL Start */
    String httpCommand;

public:
    /** @brief Constructor */
    Esp01sRelay(const String relayIpAddress, const uint8_t portId) {
        internalRelayState = E_ESP01S_RELAY_OPEN;
        httpCommand = "http://" + relayIpAddress + ":" + String(portId);
    }

    /**
     * @brief Send commands to a generic ESP-01S device
     * @details
     *  This function is used to send commands to the ESP-01S Relay using HTTP GET commands
     *
     * @param command       E_ESP01S_RELAY_OPEN or E_ESP01S_RELAY_CLOSE
     *
     * @return t_espNowErrorCodes
     */
    t_httpErrorCodes sendEsp01sRelayCommand(const t_esp01sRelayState command);

    /**
     * @brief Get the Esp01s Relay State
     * @details
     *  This function is used to retrieve the status of the ESP-01S Relay using HTTP GET commands
     *
     * @return t_esp01sRelayState
     */
    t_httpErrorCodes getEsp01sRelayState(t_esp01sRelayState * const relayState);
};

#endif /* ESP_01_S_RELAY_H */
