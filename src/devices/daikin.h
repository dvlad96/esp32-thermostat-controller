#ifndef DAIKIN_H
#define DAIKIN_H

#include "Arduino.h"
#include <WiFi.h>
#include <HTTPClient.h>

#include "errorCodes.h"

/* -------------------- Defines / Macros -------------------- */
#define DEFAULT_DAIKIN_PORT     (80U)

#define DAIKIN_POWER_OFF        (false)
#define DAIKIN_POWER_ON         (true)
#define DAIKIN_RETRY_MAX        (10)

/* -------------------- Typedef defines --------------------- */
/** @details This enum represents the type of a HTTP Request */
typedef enum {
    E_GET_CONTROL_INFO = 0,
    E_SET_CONTROL_INFO = 1
} t_requestType;

/** @details This enum represents the Daikin states */
typedef enum {
    E_MODE_AUTO = 1,
    E_MODE_DRY  = 2,
    E_MODE_COOL = 3,
    E_MODE_HEAT = 4,
    E_MODE_FAN = 6
} t_mode;

/** @details This enum represent the Fan Rate Mode and Fan Speed */
typedef enum {
    E_AUTO = 'A',
    E_SILENCE = 'B',
    E_LVL_1 = '3',
    E_LVL_2 = '4',
    E_LVL_3 = '5',
    E_LVL_4 = '6',
    E_LVL_5 = '7'
} t_fanMode;

/** @details This enum represents the Fan Direction*/
typedef enum {
    E_STOP = 0,
    E_VERTICAL = 1,
    E_HORIZONTAL = 2,
    E_ALL = 3
} t_fanDirection;

/** @details This structure is used for device status and device control */
typedef struct {
    bool retSts;                                /**< Request status */
    bool power;                                 /**< Device power status. False = off, True = on */
    t_mode mode;                                /**< Device operating mode */
    float setPointTemperature;                  /**< Device target temperature */
    int setPointHumidity;                       /**< Device target humidity */
    char fanControl;                            /**< Device Fan Mode */
    t_fanDirection fanDirection;                /**< Device Fan Swing Direction */
} t_deviceInfo;


class daikin {

private:
    String ipAddress;
    float portId;
    float targetTemperature;
    HTTPClient http;
    String urlStart;
    t_deviceInfo currentDeviceSts;

    t_httpErrorCodes getDeviceStatus(void);
public:
    daikin(char * ipAddress, const int portId) {
        this->ipAddress = ipAddress;
        this->portId = portId;
        this->urlStart = "http://" + String(ipAddress) + ":" + String(portId) + "/aircon/";
    }

    t_httpErrorCodes powerOnOff(bool power);
    t_httpErrorCodes setTemperature(t_mode mode, const float newTemperature);

};

#endif /* DAIKIN_H */