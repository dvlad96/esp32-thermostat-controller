/************************************************
 *  Includes
 ***********************************************/
#include "esp01sRelay.h"

/************************************************
 *  Defines / Macros
 ***********************************************/
/** @brief Delay time in case initialization failed */
#define ESP_NOW_INIT_RETRY_TIME         (1000U)

/** @brief Number of retires in case initialization failed */
#define ESP_NOW_NB_RETRIES              (10U)

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Static function implementation
 ***********************************************/

/************************************************
 *  Public Method Implementation
 ***********************************************/
void esp01sRelay::espNowCommunicationSetup(void) {

    uint8_t retry = 0U;
    esp_now_peer_info_t peerInfo;

    /* Initialize the ESP-Now state */
    state = E_ESP_NOW_COMMUNICATION_UP;

    /* Try to perform the setup for a max of RELAY_NB_RETRIES */
    do {
        /* Initialize ESP-Now protocol */
        if (esp_now_init() != ESP_OK) {
            Serial.println("Error initializing ESP-NOW");
            state = E_ESP_NOW_COMMUNICATION_DOWN;
        }

        if (state == E_ESP_NOW_COMMUNICATION_UP) {
            /* Register peer (ESP-01S) to send data to */
            (void)memset(&peerInfo, 0, sizeof(peerInfo));
            /* Set the channel (0-13) to match the ESP-01S */
            peerInfo.channel = channel;
            (void)memcpy(peerInfo.peer_addr, espRelayAddress, ESP_NOW_MAC_ADDRESS_SIZE);

            if (esp_now_add_peer(&peerInfo) != ESP_OK) {
                Serial.println("Failed to add peer");
                state = E_ESP_NOW_COMMUNICATION_DOWN;
            }
        }

        if (state == E_ESP_NOW_COMMUNICATION_DOWN) {
            delay(ESP_NOW_INIT_RETRY_TIME);
        }

        retry++;
    } while ((state != E_ESP_NOW_COMMUNICATION_UP) || (retry < ESP_NOW_NB_RETRIES));
}

t_espNowErrorCodes esp01sRelay::sendEspNowRelayCommand(const uint8_t * const command, const uint8_t commandSize) {

    t_espNowErrorCodes commandError = E_COMMAND_SUCCESS;

    if (esp_now_send(macAddr, command, commandSize) != ESP_OK) {
        commandError = E_COMMAND_FAILURE;
    }

    return (commandError);
}

t_espNowState esp01sRelay::getEspNowRelayStatus(void) {

    return (state);
}

/************************************************
 *  Private Method implementation
 ***********************************************/
