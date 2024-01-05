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
/** @brief Channel 0 for ESP-Now Communication protocol */
#define ESP_NOW_CHANNEL_0                   (0U)

/************************************************
 *  Typedef definition
 ***********************************************/
/** @brief ESP-Now State for the ESP-01S device */
typedef enum {
    E_ESP_NOW_COMMUNICATION_UP   = 0U,          /**< ESP-Now Communication Protocol Up */
    E_ESP_NOW_COMMUNICATION_DOWN = 1U           /**< ESP-Now Communication Protocol Down */
} t_espNowState;

/** @brief ESP-01S Relay State */
typedef enum {
    E_ESP01S_RELAY_OFF = 0U,                    /**< ESP-01S Relay OFF */
    E_ESP01S_RELAY_ON  = 1U                     /**< ESP-01S Relay ON */
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
    /** @brief ESP-Now Communication Protocol State */
    t_espNowState state;

    /** @brief ESP-01S relay state */
    t_esp01sRelayState relayState;

    /** @brief ESP-01S MAC Address */
    uint8_t macAddr[ESP_NOW_MAC_ADDRESS_SIZE];

    /** @brief ESP-Now Channel */
    uint8_t channel;

public:
    /** @brief Constructor */
    esp01sRelay(const uint8_t macAddress[ESP_NOW_MAC_ADDRESS_SIZE], const uint8_t espNowChannel) {
        (void)memcpy(macAddr, macAddress, ESP_NOW_MAC_ADDRESS_SIZE);
        state = E_ESP_NOW_COMMUNICATION_DOWN;
        relayState = E_ESP01S_RELAY_OFF;
        channel = espNowChannel;
    }

    /**
     * @brief Initialize the ESP-Now Communication protocol
     * @details
     *  This function is used to initialize the ESP-Now communication
     * protocol of a generic ESP-01S device. In case the ESP-Now fails to
     * initialize, this function will try for a number of RELAY_NB_RETRIES times
     * with a delay of INIT_RETRY_TIME in between.
    */
    void espNowCommunicationSetup(void);

    /**
     * @brief Send commands to a generic ESP-01S device
     * @details
     *  This function is used to send commands on the ESP-Now protocol
     *
     * @param command       ESP-Now command
     * @param commandSize   ESP-Now command size
     *
     * @return t_espNowErrorCodes
     */
    t_espNowErrorCodes sendEspNowRelayCommand(const uint8_t * const command, const uint8_t commandSize);

    /**
     * @brief Get the Esp Now Relay Status
     * @details
     *  This function returns the ESP-Now status
     *
     * @return t_espNowState
     */
    t_espNowState getEspNowStatus(void);

    void setEsp01sRelayState(const t_esp01sRelayState newState);

    t_esp01sRelayState getEsp01sRelayState(void);
};

#endif /* ESP_01_S_RELAY_H */
