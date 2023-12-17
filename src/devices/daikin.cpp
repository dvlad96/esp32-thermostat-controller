#include "daikin.h"
#include <tuple>

/* -------------------- Defines / Macros -------------------- */
#define HTTP_RESPONSE_NOT_OK    ("KO")

/* -------------------- Typedef defines --------------------- */

/* -------------------- Static function declaration -------------------- */
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
    } else {
        url = urlStart + "set_control_info?" + "pow=" + String(int(controlParams->power))
                                             + "&mode=" + String(controlParams->mode)
                                             + "&stemp=" + String(controlParams->setPointTemperature)
                                             + "&shum=" + String(controlParams->setPointHumidity)
                                             + "&f_rate=" + String(controlParams->fanControl)
                                             + "&f_dir=" + String(controlParams->fanDirection);

        Serial.println(url);
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

/* -------------------- Public method implementation  -------------------- */
t_httpErrorCodes daikin::powerOnOff(bool power) {
    std::tuple<t_httpErrorCodes, String> status(E_REQUEST_FAILURE, "");
    t_httpErrorCodes getStsErr;

    /* Get the current status */
    getStsErr = this->getDeviceStatus();
    if ((E_REQUEST_SUCCESS == getStsErr) &&
        (this->currentDeviceSts.retSts == true)) {

        /* Set the new power mode */
        this->currentDeviceSts.power = power;
        status = createUrlRequest(&this->http, this->urlStart, E_SET_CONTROL_INFO, &this->currentDeviceSts);
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
    getStsErr = this->getDeviceStatus();

    if ((E_REQUEST_SUCCESS == getStsErr) &&
        (this->currentDeviceSts.retSts == true)) {

        /* Set the new target temperature */
        this->currentDeviceSts.setPointTemperature = newTemperature;
        this->currentDeviceSts.mode = mode;

        /* Check if device is ON */
        if (this->currentDeviceSts.power == true) {
            status = createUrlRequest(&this->http, this->urlStart, E_SET_CONTROL_INFO, &this->currentDeviceSts);
        }

        /** @todo Remove */
        (void)this->getDeviceStatus();
    } else {
        /* Communication error */
        Serial.println("Device not responsive");
    }

    return (std::get<0>(status));
}

t_httpErrorCodes daikin::setFanSpeed(t_fanMode * speed) {

    std::tuple<t_httpErrorCodes, String> status(E_REQUEST_FAILURE, "");
    t_httpErrorCodes getStsErr;

    /* Get the current status */
    getStsErr = this->getDeviceStatus();
    if ((getStsErr == E_REQUEST_SUCCESS)  &&
        (this->currentDeviceSts.retSts == true)) {

        /* Set the new Fan Speed */
        (void)memcpy(&this->currentDeviceSts.fanControl, speed, sizeof(char));

        /* Check if device is ON */
        if (this->currentDeviceSts.power == true) {
            status = createUrlRequest(&this->http, this->urlStart, E_SET_CONTROL_INFO, &this->currentDeviceSts);
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
    getStsErr = this->getDeviceStatus();
    if ((getStsErr == E_REQUEST_SUCCESS)  &&
        (this->currentDeviceSts.retSts == true)) {

        /* Set the new Fan Swing Mode */
        this->currentDeviceSts.fanDirection = swing;

        /* Check if device is ON */
        if (this->currentDeviceSts.power == true) {
            status = createUrlRequest(&this->http, this->urlStart, E_SET_CONTROL_INFO, &this->currentDeviceSts);
        }
    } else {
        /* Communication error */
        Serial.println("Device not responsive");
    }

    return (std::get<0>(status));
}

/* -------------------- Private method implementation  -------------------- */
t_httpErrorCodes daikin::getDeviceStatus(void) {

    std::tuple<t_httpErrorCodes, String> status(E_REQUEST_FAILURE, "");

    status = createUrlRequest(&this->http, this->urlStart, E_GET_CONTROL_INFO);
    unwrapDeviceStatus(std::get<1>(status), &this->currentDeviceSts);

    Serial.print("Return status: ");
    Serial.println(this->currentDeviceSts.retSts);
    Serial.print("Device status: ");
    Serial.println(this->currentDeviceSts.power);
    Serial.print("Device mode: ");
    Serial.println(this->currentDeviceSts.mode);
    Serial.print("Device target temperature: ");
    Serial.println(this->currentDeviceSts.setPointTemperature);
    Serial.print("Device target humidity: ");
    Serial.println(this->currentDeviceSts.setPointHumidity);
    Serial.print("Device fan mode: ");
    Serial.println(this->currentDeviceSts.fanControl);
    Serial.print("Device fan direction: ");
    Serial.println(this->currentDeviceSts.fanDirection);
    Serial.println("===========");

    return (std::get<0>(status));
}
