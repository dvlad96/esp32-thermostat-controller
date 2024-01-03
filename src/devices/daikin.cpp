/************************************************
 *  Includes
 ***********************************************/
#include <tuple>

/* Local files */
#include "daikin.h"

/************************************************
 *  Defines / Macros
 ***********************************************/
/** @brief This define represents a NOT OK response of a HTTP Request*/
#define HTTP_RESPONSE_NOT_OK                ("KO")

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Static function implementation
 ***********************************************/
static std::tuple<t_httpErrorCodes, String> getHttpResponse(HTTPClient * http, String url) {

    t_httpErrorCodes error;
    String response = String(HTTP_RESPONSE_NOT_OK);
    int httpCode;

    http->begin(url);
    httpCode = http->GET();
    if (httpCode > 0U) {
        if (httpCode == HTTP_CODE_OK) {
            error = E_REQUEST_SUCCESS;
            response = http->getString();
        } else {
          error = E_REQUEST_FAILURE;
        }
    } else {
        error = E_REQUEST_FAILURE;
    }

    return std::make_tuple(error, response);
}

static std::tuple<t_httpErrorCodes, String> createUrlRequest(HTTPClient * http, String urlStart, t_requestType requestType, t_deviceInfo * controlParams = NULL) {

    String url;
    String l_response;
    t_httpErrorCodes error;
    int httpCode;
    std::tuple<t_httpErrorCodes, String> status;

    if (requestType == E_GET_CONTROL_INFO) {
        url = urlStart + "get_control_info?";
    } else if (requestType == E_SET_CONTROL_INFO) {
        url = urlStart + "set_control_info?" + "pow=" + String(int(controlParams->power))
                                             + "&mode=" + String(controlParams->mode)
                                             + "&stemp=" + String(controlParams->setPointTemperature)
                                             + "&shum=" + String(controlParams->setPointHumidity)
                                             + "&f_rate=" + String(controlParams->fanControl)
                                             + "&f_dir=" + String(controlParams->fanDirection);

        Serial.println(url);
    } else if (requestType == E_GET_SENSOR_INFO) {
        url = urlStart + "get_sensor_info";
    }

    status = getHttpResponse(http, url);

    return (status);
}

static void unwrapDeviceStatus(String status, t_deviceInfo * const deviceInfo) {

    char * token;
    char * savePtr;
    char * mutableInputString = strdup(status.c_str());
    token = strtok_r(mutableInputString, ",", &savePtr);

    while (token != NULL) {
        // Split each token into a key and value pair
        char* key = strtok(token, "=");
        char* value = strtok(NULL, "=");

        if (key != NULL && value != NULL) {
            // Determine which element of the struct to update based on the key
            if (strcmp(key, "ret") == 0) {
                deviceInfo->retSts = (strcmp(value, "OK") == 0);
            } else if (strcmp(key, "pow") == 0) {
                deviceInfo->power = (strcmp(value, "1") == 0);
            } else if (strcmp(key, "mode") == 0) {
                deviceInfo->mode = t_mode(std::stoi(value));
            } else if (strcmp(key, "stemp") == 0) {
                deviceInfo->setPointTemperature = t_mode(std::stoi(value));
            } else if (strcmp(key, "shum") == 0) {
                deviceInfo->setPointHumidity = t_mode(std::stoi(value));
            } else if (strcmp(key, "f_rate") == 0) {
                strcpy(&deviceInfo->fanControl, value);
            } else if (strcmp(key, "f_dir") == 0) {
                deviceInfo->fanDirection = t_fanDirection(std::stoi(value));
            }
            else {
                /* The rest of the response is not needed */
            }
        }
        /* Get the next token */
        token = strtok_r(NULL, ",", &savePtr);
    }

    free(mutableInputString);
}

static void unwrapSensorStatus(String status, t_sensorInfo * const sensorInfo) {

    char * token;
    char * savePtr;
    char * mutableInputString = strdup(status.c_str());
    token = strtok_r(mutableInputString, ",", &savePtr);

    while (token != NULL) {
        // Split each token into a key and value pair
        char* key = strtok(token, "=");
        char* value = strtok(NULL, "=");

        if (key != NULL && value != NULL) {
            // Determine which element of the struct to update based on the key
            if (strcmp(key, "ret") == 0) {
                sensorInfo->retSts = (strcmp(value, "OK") == 0);
            } else if (strcmp(key, "htemp") == 0) {
                sensorInfo->htemp = std::atof(value);
            } else if (strcmp(key, "hhum") == 0) {
                sensorInfo->hhum = std::atof(value);
            } else if (strcmp(key, "otemp") == 0) {
                sensorInfo->otemp = std::atof(value);
            } else if (strcmp(key, "err") == 0) {
                sensorInfo->err = bool(std::stoi(value));
            } else if (strcmp(key, "cmpfreq") == 0) {
                sensorInfo->cmpfreq = std::stoi(value);
            } else {
                /* The rest of the response is not needed */
            }
        }
        /* Get the next token */
        token = strtok_r(NULL, ",", &savePtr);
    }

    free(mutableInputString);
}

/************************************************
 *  Public Method Implementation
 ***********************************************/
t_httpErrorCodes daikin::powerOnOff(bool power) {
    std::tuple<t_httpErrorCodes, String> status(E_REQUEST_FAILURE, "");
    t_httpErrorCodes getStsErr;

    /* Get the current status */
    getStsErr = getDeviceStatus();
    if ((E_REQUEST_SUCCESS == getStsErr) &&
        (currentDeviceSts.retSts == true)) {

        /* Set the new power mode */
        currentDeviceSts.power = power;
        acPowerState = power;
        status = createUrlRequest(&http, urlStart, E_SET_CONTROL_INFO, &currentDeviceSts);
    } else {
        /* Communication error */
        Serial.println("Device not responsive");
    }

    return (std::get<0>(status));
}

t_httpErrorCodes daikin::setTemperature(t_mode mode, const float newTemperature) {

    std::tuple<t_httpErrorCodes, String> status(E_REQUEST_FAILURE, "");
    t_httpErrorCodes getStsErr;

    /* Get the current status */
    getStsErr = getDeviceStatus();

    if ((E_REQUEST_SUCCESS == getStsErr) &&
        (currentDeviceSts.retSts == true)) {

        /* Set the new target temperature */
        currentDeviceSts.setPointTemperature = newTemperature;
        currentDeviceSts.mode = mode;

        /* Check if device is ON */
        if (currentDeviceSts.power == true) {
            status = createUrlRequest(&http, urlStart, E_SET_CONTROL_INFO, &currentDeviceSts);
        }

        /** @todo Remove */
        (void)getDeviceStatus();
    } else {
        /* Communication error */
        Serial.println("Device not responsive");
    }

    return (std::get<0>(status));
}

t_httpErrorCodes daikin::setFanSpeed(t_fanMode speed) {

    std::tuple<t_httpErrorCodes, String> status(E_REQUEST_FAILURE, "");
    t_httpErrorCodes getStsErr;

    /* Get the current status */
    getStsErr = getDeviceStatus();
    if ((getStsErr == E_REQUEST_SUCCESS)  &&
        (currentDeviceSts.retSts == true)) {

        /* Set the new Fan Speed */
        (void)memset(&currentDeviceSts.fanControl, speed, sizeof(char));

        /* Check if device is ON */
        if (currentDeviceSts.power == true) {
            status = createUrlRequest(&http, urlStart, E_SET_CONTROL_INFO, &currentDeviceSts);
        }
    } else {
        /* Communication error */
        Serial.println("Device not responsive");
    }

    return (std::get<0>(status));
}

t_httpErrorCodes daikin::setFanSwingMode(t_fanDirection swing) {

    std::tuple<t_httpErrorCodes, String> status(E_REQUEST_FAILURE, "");
    t_httpErrorCodes getStsErr;

    /* Get the current status */
    getStsErr = getDeviceStatus();
    if ((getStsErr == E_REQUEST_SUCCESS)  &&
        (currentDeviceSts.retSts == true)) {

        /* Set the new Fan Swing Mode */
        currentDeviceSts.fanDirection = swing;

        /* Check if device is ON */
        if (currentDeviceSts.power == true) {
            status = createUrlRequest(&http, urlStart, E_SET_CONTROL_INFO, &currentDeviceSts);
        }
    } else {
        /* Communication error */
        Serial.println("Device not responsive");
    }

    return (std::get<0>(status));
}

t_httpErrorCodes daikin::getCurrentTemperature(float * const sensorTemperature) {

    std::tuple<t_httpErrorCodes, String> status(E_REQUEST_FAILURE, "");
    *sensorTemperature = 0;

    status = createUrlRequest(&http, urlStart, E_GET_SENSOR_INFO);
    if (std::get<0>(status) == E_REQUEST_SUCCESS) {
        unwrapSensorStatus(std::get<1>(status), &currentSensorInfo);

        if (currentSensorInfo.retSts == true) {
            *sensorTemperature = currentSensorInfo.htemp;
        }
    }

    return (std::get<0>(status));
}

bool daikin::getPowerState(void) {

    return (acPowerState);
}

/************************************************
 *  Private Method implementation
 ***********************************************/
t_httpErrorCodes daikin::getDeviceStatus(void) {

    std::tuple<t_httpErrorCodes, String> status(E_REQUEST_FAILURE, "");

    status = createUrlRequest(&http, urlStart, E_GET_CONTROL_INFO);
    unwrapDeviceStatus(std::get<1>(status), &currentDeviceSts);

    Serial.print("Return status: ");
    Serial.println(currentDeviceSts.retSts);
    Serial.print("Device status: ");
    Serial.println(currentDeviceSts.power);
    Serial.print("Device mode: ");
    Serial.println(currentDeviceSts.mode);
    Serial.print("Device target temperature: ");
    Serial.println(currentDeviceSts.setPointTemperature);
    Serial.print("Device target humidity: ");
    Serial.println(currentDeviceSts.setPointHumidity);
    Serial.print("Device fan mode: ");
    Serial.println(currentDeviceSts.fanControl);
    Serial.print("Device fan direction: ");
    Serial.println(currentDeviceSts.fanDirection);
    Serial.println("===========");

    return (std::get<0>(status));
}
