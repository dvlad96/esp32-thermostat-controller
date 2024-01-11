/************************************************
 *  Includes
 ***********************************************/
#include <string>
#include "HomeSpan.h"

/* Local files */
#include "esp01sRelay.h"

/************************************************
 *  Defines / Macros
 ***********************************************/
/** @brief Delay time in case initialization failed */
#define ESP_NOW_INIT_RETRY_TIME         (1000U)

/** @brief Number of retires in case initialization failed */
#define ESP_NOW_NB_RETRIES              (10U)

/** @brief HTTP Success code */
#define HTTP_RESPONSE_SUCCESS           (200)

/** @brief HTTP BAD Request code */
#define HTTP_RESPONSE_BAD_REQUEST       (400)

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Static function implementation
 ***********************************************/

/************************************************
 *  Public Method Implementation
 ***********************************************/
t_httpErrorCodes Esp01sRelay::sendEsp01sRelayCommand(const t_esp01sRelayState command) {

    HTTPClient http;
    String url = httpCommand;
    t_httpErrorCodes error = E_REQUEST_FAILURE;

    if (command == E_ESP01S_RELAY_OPEN) {
        url += String(RELAY_OPEN_COMMAND);
    } else {
        url += String(RELAY_CLOSE_COMMAND);
    }

    WEBLOG("Sending the command: %s to the relay\n", url.c_str());

    if (http.begin(url) == true) {
        int httpCode = http.GET();
        if (httpCode == HTTP_RESPONSE_SUCCESS) {
            WEBLOG("Relay request success");
            error = E_REQUEST_SUCCESS;
        } else {
            WEBLOG("Tried to send a command and client responded with HTTP Code: %d\n", httpCode);
        }
        http.end();
    } else {
        WEBLOG("Failed to begin HTTP request");
    }

    return (error);
}

t_httpErrorCodes Esp01sRelay::getEsp01sRelayState(t_esp01sRelayState * const relayState) {

    HTTPClient http;
    String url = httpCommand + "/relay_status";
    t_httpErrorCodes error = E_REQUEST_FAILURE;
    uint8_t response;
    int httpCode;

    if (http.begin(url) == true) {
        httpCode = http.GET();
        if (httpCode == HTTP_RESPONSE_SUCCESS) {
            response = std::stoul(http.getString().c_str());

            internalRelayState = (t_esp01sRelayState)response;
            *relayState = internalRelayState;
            error = E_REQUEST_SUCCESS;
        } else {
           WEBLOG("Failed to retrieve relay status\n");
        }
        http.end();
    } else {
        WEBLOG("Failed to begin HTTP request");
    }

    return (error);
}
/************************************************
 *  Private Method implementation
 ***********************************************/
