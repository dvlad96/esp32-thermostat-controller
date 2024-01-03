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

#define M_SET_FAN_SPEED(speed)      (                   \
    (speed == 0) ? E_AUTO :                             \
    (speed > 0  && speed <= 10)  ? E_SILENCE :          \
    (speed > 10 && speed <= 40)  ? E_LVL_1 :            \
    (speed > 40 && speed <= 60)  ? E_LVL_2 :            \
    (speed > 60 && speed <= 80)  ? E_LVL_3 :            \
    (speed > 80 && speed <= 100) ? E_LVL_4 : E_LVL_5    \
)

/************************************************
 *  Typedef definition
 ***********************************************/
/** @brief This enum represents the type of a HTTP Request */
typedef enum {
    E_GET_CONTROL_INFO = 0U,        /**< Request to read AC active parameters */
    E_SET_CONTROL_INFO = 1U,        /**< Request to modify AC active parameters */
    E_GET_SENSOR_INFO  = 2U
} t_requestType;

/** @brief This enum represents the Daikin states */
typedef enum {
    E_MODE_AUTO = 1U,               /**< AUTO mode */
    E_MODE_DRY  = 2U,               /**< Dehumidify mode */
    E_MODE_COOL = 3U,               /**< COOL mode */
    E_MODE_HEAT = 4U,               /**< HEAT mode */
    E_MODE_FAN  = 6U                /**< Fan mode */
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
    E_STOP       = 0U,              /**< Fan Stop */
    E_VERTICAL   = 1U,              /**< Vertical Swing */
    E_HORIZONTAL = 2U,              /**< Horizontal Swing */
    E_ALL        = 3U               /**< Swing in all directions */
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

typedef struct {
    bool retSts;
    float htemp;
    float hhum;
    float otemp;
    bool err;
    int cmpfreq;
} t_sensorInfo;

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
    t_sensorInfo currentSensorInfo;
    bool acPowerState;  // true = on

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
        acPowerState = false;
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
    t_httpErrorCodes setFanSpeed(t_fanMode speed);

    /**
     * @brief Daikin Set Fan Swing Mode public method
     * @details
     *  This method is responsible for changing the target fan swing mode of the Daikin AC
     *
     * @param swing     New swing mode
     * @return t_httpErrorCodes
     */
    t_httpErrorCodes setFanSwingMode(t_fanDirection swing);

    /* https://github.com/ael-code/daikin-control */
    t_httpErrorCodes getCurrentTemperature(float * const sensorTemperature);

    bool getPowerState(void);

};

#endif /* DAIKIN_H */
