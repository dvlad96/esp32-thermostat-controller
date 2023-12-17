#ifndef DAIKIN_H
#define DAIKIN_H

/************************************************
 *  Includes
 ***********************************************/
#include "Arduino.h"
#include <WiFi.h>
#include <HTTPClient.h>

/* Local files */
#include "errorCodes.h"

/************************************************
 *  Defines / Macros
 ***********************************************/
/** @brief Default Daikin AC Port ID */
#define DEFAULT_DAIKIN_PORT         (80U)

/** @brief This define is used for AC Power OFF */
#define DAIKIN_POWER_OFF            (false)

/** @brief This define is used for AC Power ON */
#define DAIKIN_POWER_ON             (true)

/** @brief Maximum number of retries until timeout */
#define DAIKIN_RETRY_MAX            (10U)

/************************************************
 *  Typedef definition
 ***********************************************/
/** @brief This enum represents the type of a HTTP Request */
typedef enum {
    E_GET_CONTROL_INFO = 0,         /**< Request to read AC active parameters */
    E_SET_CONTROL_INFO = 1          /**< Request to modify AC active parameters */
} t_requestType;

/** @brief This enum represents the Daikin states */
typedef enum {
    E_MODE_AUTO = 1,                /**< AUTO mode */
    E_MODE_DRY  = 2,                /**< Dehumidify mode */
    E_MODE_COOL = 3,                /**< COOL mode */
    E_MODE_HEAT = 4,                /**< HEAT mode */
    E_MODE_FAN  = 6                 /**< Fan mode */
} t_mode;

/** @brief This enum represent the Fan Rate Mode and Fan Speed */
typedef enum {
    E_AUTO    = 'A',                /**< AUTO mode */
    E_SILENCE = 'B',                /**< SILENCE mode */
    E_LVL_1   = '3',                /**< Fan Speed LVL 1 */
    E_LVL_2   = '4',                /**< Fan Speed LVL 2 */
    E_LVL_3   = '5',                /**< Fan Speed LVL 3 */
    E_LVL_4   = '6',                /**< Fan Speed LVL 4 */
    E_LVL_5   = '7'                 /**< Fan Speed LVL 5 */
} t_fanMode;

/** @brief This enum represents the Fan Direction*/
typedef enum {
    E_STOP       = 0,               /**< Fan Stop */
    E_VERTICAL   = 1,               /**< Vertical Swing */
    E_HORIZONTAL = 2,               /**< Horizontal Swing */
    E_ALL        = 3                /**< Swing in all directions */
} t_fanDirection;

/** @brief This structure is used for device status and device control */
typedef struct {
    bool retSts;                    /**< Request status */
    bool power;                     /**< Device power status. False = off, True = on */
    t_mode mode;                    /**< Device operating mode */
    float setPointTemperature;      /**< Device target temperature */
    int setPointHumidity;           /**< Device target humidity */
    char fanControl;                /**< Device Fan Mode */
    t_fanDirection fanDirection;    /**< Device Fan Swing Direction */
} t_deviceInfo;

/************************************************
 *  Class definition
 ***********************************************/
/**
 * @brief Daikin class definition.
 * @details This class is used to control a Daikin AC unit which is connected to the
 * Internet and uses HTTP Requests for control purposes.
 */
class daikin {
private:
    HTTPClient http;
    String urlStart;
    t_deviceInfo currentDeviceSts;

    /**
     * @brief Daikin Get Device Status private method
     * @details
     *  This private method is responsible to get the active parameters of the
     * Daikin AC and to store them in the @param currentDeviceSts class attribute.
     *
     * @return t_httpErrorCodes
     */
    t_httpErrorCodes getDeviceStatus(void);

public:
    /**
     * @brief Constructor of the Daikin class
     *
     * @param ipAddress     Daikin device IP Address
     * @param portId        Daikin device Port ID
     */
    daikin(char * ipAddress, const int portId) {
        this->urlStart = "http://" + String(ipAddress) + ":" + String(portId) + "/aircon/";
    }

    /**
     * @brief Daikin Power ON/OFF public method
     * @details
     *  This method is responsible for powering ON or OFF the Daikin AC
     *
     * @param power     true = ON, false = OFF
     * @return t_httpErrorCodes
     */
    t_httpErrorCodes powerOnOff(bool power);

    /**
     * @brief Daikin Set Temperature public method
     * @details
     *  This method is responsible for changing the target temperature and targe fan mode
     * of the Daikin AC
     *
     * @param mode              Desired mode change
     * @param newTemperature    Desired temperature change
     * @return t_httpErrorCodes
     */
    t_httpErrorCodes setTemperature(t_mode mode, const float newTemperature);

    /**
     * @brief Daikin Set Fan Speed public method
     * @details
     *  This method is responsible for changing the target fan speed of the Daikin AC
     *
     * @param speed     New speed
     * @return t_httpErrorCodes
     */
    t_httpErrorCodes setFanSpeed(t_fanMode * speed);

    /**
     * @brief Daikin Set Fan Swing Mode public method
     * @details
     *  This method is responsible for changing the target fan swing mode of the Daikin AC
     *
     * @param swing     New swing mode
     * @return t_httpErrorCodes
     */
    t_httpErrorCodes setFanSwingMode(t_fanDirection swing);
};

#endif /* DAIKIN_H */
