#ifndef HEATING_RELAY_H
#define HEATING_RELAY_H

/************************************************
 *  Includes
 ***********************************************/
#include "esp01sRelay.h"
#include "private/espNowInfo.h"

/************************************************
 *  Defines / Macros
 ***********************************************/
/** @brief Heating ON command */
#define RELAY_COMMAND_ON            (0x1U)

/** @brief Heating OFF command */
#define RELAY_COMMAND_OFF           (0x0U)

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Public function definition
 ***********************************************/
/**
 * @brief Heating Relay class definition.
 * @details This class is used to define the Heating Relay device
 */
class heatingRelay : public esp01sRelay {
private:
public:
    heatingRelay(void) : esp01sRelay(espRelayAddress, ESP_NOW_CHANNEL_0) {
        /* Nothing to initialize */
    }

    /**
     * @brief Send commands to the Heating Relay public function
     * @details This function is used to send ON/OFF commands to the Heating Relay
     *
     * @param command    RELAY_COMMAND_ON or RELAY_COMMAND_OFF
     *
     * @return t_espNowErrorCodes
     */
    t_espNowErrorCodes sendRelayCommand(const uint8_t command);
};

#endif /* HEATING_RELAY_H */