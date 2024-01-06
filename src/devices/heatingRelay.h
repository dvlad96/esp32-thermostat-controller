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
    heatingRelay(void) : esp01sRelay("192.168.1.100", "80") {
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
    t_espNowErrorCodes sendRelayCommand(const t_esp01sRelayState command);
};

#endif /* HEATING_RELAY_H */