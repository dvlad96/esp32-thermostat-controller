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

#define HTTP_RESPONSE_SUCCESS           (200)
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

t_httpErrorCodes esp01sRelay::sendEsp01sRelayCommand(const String command) {

    String url = httpCommand + command;
    int httpCode;
    t_httpErrorCodes error = E_REQUEST_FAILURE;

    http->begin(url);
    httpCode = http->GET();
    if (httpCode == HTTP_RESPONSE_SUCCESS) {
        error = E_REQUEST_SUCCESS;
    } else {
        Serial.printf("Tried to send %s and client responded with HTTP Code: %d\n", command.c_str(), httpCode);
    }

    http->end();

    return (error);
}

t_esp01sRelayState esp01sRelay::getEsp01sRelayState(void) {

    String url = httpCommand + "/relay_status";
    String response;
    int httpCode;

    http->begin(url);
    httpCode = http->GET();
    if (httpCode == HTTP_RESPONSE_SUCCESS) {
        error = E_REQUEST_SUCCESS;
        response = http.getString();

        internalRelayState = (t_esp01sRelayState)std::stoul(response);
    } else {
        Serial.printf("Failed to retrieve relay status\n");
    }

    http->end();

    return (internalRelayState);

}
/************************************************
 *  Private Method implementation
 ***********************************************/
