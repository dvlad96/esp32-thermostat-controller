#ifndef THERMOSTAT_H
#define THERMOSTAT_H

/************************************************
 *  Includes
 ***********************************************/
#include "HomeSpan.h"

/* Local files */
#include "devices/daikin.h"
#include "devices/heatingRelay.h"
#include "homeKitAccessories/tempHumSensor.h"

/************************************************
 *  Defines / Macros
 ***********************************************/
#define THERMOSTAT_DEFAULT_TARGET_TEMPERATURE       (22U)
#define THERMOSTAT_DEFAULT_TARGET_HUMIDITY          (50U)

/************************************************
 *  Typedef definition
 ***********************************************/
/** @brief This enum represents all the possible states of a Home Kit Thermostat */
typedef enum {
    E_THERMOSTAT_STATE_OFF      = 0U,           /**< Thermostat OFF */
    E_THERMOSTAT_STATE_HEAT     = 1U,           /**< Thermostat set on HEAT mode */
    E_THERMOSTAT_STATE_COOL     = 2U,           /**< Thermostat set on COOL mode */
    E_THERMOSTAT_STATE_AUTO     = 3U            /**< Thermostat set on AUTO mode */
} t_thermostatStates;

/************************************************
 *  Class definition
 ***********************************************/
struct HS_Thermostat : Service::Thermostat, daikin, HS_TempHumSensor {
private:
    t_httpErrorCodes powerOnOffDaikin(bool power) {
        t_httpErrorCodes daikinError;
        uint8_t retry = 0;

        do {
            daikinError = this->powerOnOff(power);
            retry++;

            /* Wait 1 second before next retry */
            delay(1000);
        } while ((daikinError != E_REQUEST_SUCCESS) || (retry > DAIKIN_RETRY_MAX));

        if (daikinError == E_REQUEST_FAILURE) {
            Serial.println("Can not turn on/off Daikin");
        }

        return (daikinError);
    }

public:
    // Create characteristics, set initial values, and set storage in NVS to true
    Characteristic::CurrentHeatingCoolingState currentState{0, true};
    Characteristic::TargetHeatingCoolingState targetState{0, true};
    Characteristic::CurrentTemperature currentTemp{THERMOSTAT_DEFAULT_TARGET_TEMPERATURE, true};
    Characteristic::TargetTemperature targetTemp{THERMOSTAT_DEFAULT_TARGET_TEMPERATURE, true};
    Characteristic::CurrentRelativeHumidity currentHumidity{THERMOSTAT_DEFAULT_TARGET_TEMPERATURE, true};
    Characteristic::TargetRelativeHumidity targetHumidity{THERMOSTAT_DEFAULT_TARGET_TEMPERATURE, true};
    Characteristic::HeatingThresholdTemperature heatingThreshold{22, true};
    Characteristic::CoolingThresholdTemperature coolingThreshold{22, true};
    Characteristic::TemperatureDisplayUnits displayUnits{0, true}; // this is for changing the display on the actual thermostat (if any), NOT in the Home App

    HS_Thermostat(char * daikinIpAddress, const int daikinPortId,
                  const uint8_t dhtPin, const uint8_t dhtType, const uint32_t dhtPollingTime) :
                        Service::Thermostat(),
                        daikin(daikinIpAddress, daikinPortId),
                        HS_TempHumSensor(dhtPin, dhtType, dhtPollingTime) {

        Serial.printf("\n*** Creating HomeSpan Thermostat***\n");

        /* Set the initial values */
        /* Temperature */
        currentTemp.setVal(getTemperature());
        targetTemp.setVal(THERMOSTAT_DEFAULT_TARGET_TEMPERATURE);
        /* Humidity */
        currentHumidity.setVal(getHumidity());
        targetHumidity.setVal(THERMOSTAT_DEFAULT_TARGET_HUMIDITY);

    }

    boolean update() override {
        t_httpErrorCodes daikinError;
        float initialTemperature;

        if (targetState.updated()) {
            switch (targetState.getNewVal()) {
                case E_THERMOSTAT_STATE_OFF:
                    Serial.printf("Thermostat turning OFF\n");
                    daikinError = powerOnOffDaikin(DAIKIN_POWER_OFF);
                    relayOnOff(HEATING_OFF);
                    break;
                case E_THERMOSTAT_STATE_HEAT:
                    initialTemperature = targetTemp.getVal<float>();
                    Serial.printf("Thermostat set to HEAT at %s\n", temp2String(initialTemperature).c_str());
                    relayOnOff(HEATING_ON);
                    daikinError = powerOnOffDaikin(DAIKIN_POWER_OFF);
                    break;
                case E_THERMOSTAT_STATE_COOL:
                    initialTemperature = targetTemp.getVal<float>();
                    Serial.printf("Thermostat set to COOL at %s\n", temp2String(initialTemperature).c_str());
                    daikinError = this->powerOnOffDaikin(DAIKIN_POWER_ON);
                    relayOnOff(HEATING_OFF);
                    if (daikinError == E_REQUEST_SUCCESS) {
                        daikinError = setTemperature(E_MODE_COOL, initialTemperature);
                    }
                    break;
                case E_THERMOSTAT_STATE_AUTO:
                    Serial.printf("Thermostat set to AUTO from %s to %s\n", temp2String(heatingThreshold.getVal<float>()).c_str(), temp2String(coolingThreshold.getVal<float>()).c_str());
                    break;
            }
        }

        if (heatingThreshold.updated() || coolingThreshold.updated()) {
            Serial.printf("Temperature range changed to %s to %s\n", temp2String(heatingThreshold.getNewVal<float>()).c_str(), temp2String(coolingThreshold.getNewVal<float>()).c_str());
        } else if (targetTemp.updated()) {
            Serial.printf("Temperature target changed to %s\n", temp2String(targetTemp.getNewVal<float>()).c_str());
            switch (currentState.getVal()) {
                case E_THERMOSTAT_STATE_HEAT:
                    break;

                case E_THERMOSTAT_STATE_COOL:
                    daikinError = setTemperature(E_MODE_COOL, targetTemp.getNewVal<float>());
                    break;

                case E_THERMOSTAT_STATE_AUTO:
                    break;

                case E_THERMOSTAT_STATE_OFF:
                default:
                    break;
            }
        }

        if (displayUnits.updated()) {
            Serial.printf("Display Units changed to %c\n", displayUnits.getNewVal() ? 'F' : 'C');
        }

        if (targetHumidity.updated()) {
            Serial.printf("Humidity target changed to %d%%\n", targetHumidity.getNewVal());
        }

        return (true);
    }

    // This optional function makes it easy to display temperatures on the serial monitor in either F or C depending on TemperatureDisplayUnits
    String temp2String(float temp) {
        String t = displayUnits.getVal() ? String(round(temp * 1.8 + 32.0)) : String(temp);
        t += displayUnits.getVal() ? " F" : " C";
        return (t);
    }

    void loop() override {

        t_httpErrorCodes daikinError;

        switch (targetState.getVal()) {
            case E_THERMOSTAT_STATE_OFF:
                if (currentState.getVal() != 0) {
                    Serial.printf("Thermostat OFF\n");
                    currentState.setVal(0);
                }
                break;

            case E_THERMOSTAT_STATE_HEAT:
                if (currentTemp.getVal<float>() < targetTemp.getVal<float>() && currentState.getVal() != 1) {
                    Serial.printf("Turning HEAT ON\n");
                    currentState.setVal(1);
                } else if (currentTemp.getVal<float>() >= targetTemp.getVal<float>() && currentState.getVal() == 1) {
                    Serial.printf("Turning HEAT OFF\n");
                    currentState.setVal(0);
                } else if (currentState.getVal() == 2) {
                    Serial.printf("Turning COOL OFF\n");
                    currentState.setVal(0);
                }
                break;

            case E_THERMOSTAT_STATE_COOL:
                if (currentTemp.getVal<float>() > targetTemp.getVal<float>() && currentState.getVal() != 2) {
                    Serial.printf("Turning COOL ON\n");
                    daikinError = this->powerOnOffDaikin(DAIKIN_POWER_ON);
                    if (daikinError == E_REQUEST_SUCCESS) {
                        daikinError = this->setTemperature(E_MODE_COOL, targetTemp.getVal<float>());
                        currentState.setVal(2);
                    } else {
                        /* Could not get a response from the cooling device */
                    }
                } else if (currentTemp.getVal<float>() <= targetTemp.getVal<float>() && currentState.getVal() == 2) {
                    Serial.printf("Turning COOL OFF\n");
                    daikinError = this->powerOnOffDaikin(DAIKIN_POWER_OFF);
                    if (daikinError == E_REQUEST_SUCCESS) {
                        currentState.setVal(0);
                    } else {
                        /* Could not get a response from the cooling device */
                    }
                } else if (currentState.getVal() == 1) {
                    Serial.printf("Turning HEAT OFF\n");
                    /** @todo Add heating devices */
                    currentState.setVal(0);
                }
                break;

            case E_THERMOSTAT_STATE_AUTO:
                if (currentTemp.getVal<float>() < heatingThreshold.getVal<float>() && currentState.getVal() != 1) {
                    Serial.printf("Turning HEAT ON\n");
                    currentState.setVal(1);
                } else if (currentTemp.getVal<float>() >= heatingThreshold.getVal<float>() && currentState.getVal() == 1) {
                    Serial.printf("Turning HEAT OFF\n");
                    currentState.setVal(0);
                }

                if (currentTemp.getVal<float>() > coolingThreshold.getVal<float>() && currentState.getVal() != 2) {
                    Serial.printf("Turning COOL ON\n");
                    currentState.setVal(2);
                } else if (currentTemp.getVal<float>() <= coolingThreshold.getVal<float>() && currentState.getVal() == 2) {
                    Serial.printf("Turning COOL OFF\n");
                    currentState.setVal(0);
                }
                break;
        }
    }
};

#endif /* THERMOSTAT_H */
