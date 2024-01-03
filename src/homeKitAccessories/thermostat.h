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
struct HS_Thermostat : Service::Thermostat {
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
    Characteristic::TemperatureDisplayUnits displayUnits{0, true};

    DHT tempHumSensor {DHT_PIN, DHT_TYPE};
    daikin coolingDevice {(char *)DAIKIN_IP_ADDRESS, DAIKIN_PORT_ID};

    HS_Thermostat() : Service::Thermostat() {

        /* Initialize DHT sensor */
        tempHumSensor.begin();

        /* Set the initial values */
        currentTemp.setVal(tempHumSensor.readTemperature());
        currentTemp.setRange(DHT_TEMPERATURE_DEFAULT_MIN_VAL, DHT_TEMPERATURE_DEFAULT_MAX_VAL);
        targetTemp.setVal(THERMOSTAT_DEFAULT_TARGET_TEMPERATURE);
        targetTemp.setRange(DHT_TEMPERATURE_DEFAULT_MIN_VAL, DHT_TEMPERATURE_DEFAULT_MAX_VAL);

        currentHumidity.setVal(tempHumSensor.readHumidity());
        currentHumidity.setRange(DHT_HUMIDITY_DEFAULT_MIN_RANGE, DHT_HUMIDITY_DEFAULT_MAX_RANGE);
        targetHumidity.setVal(THERMOSTAT_DEFAULT_TARGET_HUMIDITY);
        currentHumidity.setRange(DHT_HUMIDITY_DEFAULT_MIN_RANGE, DHT_HUMIDITY_DEFAULT_MAX_RANGE);

        heatingThreshold.setRange(0, 30);
        coolingThreshold.setRange(0, 30);
    }

    boolean update() override {
        if (targetState.updated()) {
            switch(targetState.getNewVal()) {
                case E_THERMOSTAT_STATE_OFF:
                    Serial.printf("Thermostat turning OFF\n");

                    if (coolingDevice.getPowerState() == DAIKIN_POWER_ON) {
                        (void)powerOnOffDaikin(DAIKIN_POWER_OFF);
                    } else {
                        /* AC already turned OFF */
                    }
                    break;

                case E_THERMOSTAT_STATE_HEAT:
                    Serial.printf("Thermostat set to HEAT at %s\n",
                                   temp2String(targetTemp.getVal<float>()).c_str());

                    if (coolingDevice.getPowerState() == DAIKIN_POWER_ON) {
                        (void)powerOnOffDaikin(DAIKIN_POWER_OFF);
                    } else {
                        /* AC already turned OFF */
                    }
                    break;

                case E_THERMOSTAT_STATE_COOL:
                    Serial.printf("Thermostat set to COOL at %s\n",
                                  temp2String(targetTemp.getVal<float>()).c_str());
                    if (coolingDevice.getPowerState() == DAIKIN_POWER_OFF) {
                        (void)powerOnOffDaikin(DAIKIN_POWER_ON);
                    } else {
                        /* AC already turned OFF */
                    }
                    break;

                case E_THERMOSTAT_STATE_AUTO:
                    Serial.printf("Thermostat set to AUTO from %s to %s\n",
                                  temp2String(heatingThreshold.getVal<float>()).c_str(),temp2String(coolingThreshold.getVal<float>()).c_str());
                    break;
            }
        }

        if (heatingThreshold.updated() || coolingThreshold.updated()) {
            Serial.printf("Temperature range changed to %s to %s\n",
                          temp2String(heatingThreshold.getNewVal<float>()).c_str(), temp2String(coolingThreshold.getNewVal<float>()).c_str());
        }

        else if (targetTemp.updated()) {
            Serial.printf("Temperature target changed to %s\n",
                           temp2String(targetTemp.getNewVal<float>()).c_str());

            /* Update the new value only if the AC is on */
            if ((currentState.getVal() == E_THERMOSTAT_STATE_COOL) &&
                (true == coolingDevice.getPowerState())) {
                /* Set the new AC target temperature */
                (void)coolingDevice.setTemperature(E_MODE_COOL, targetTemp.getNewVal<float>());
            } else {
                /* Do nothing */
            }
        }

        if (displayUnits.updated()) {
            Serial.printf("Display Units changed to %c\n",displayUnits.getNewVal()?'F':'C');
        }

        if (targetHumidity.updated()) {
            Serial.printf("Humidity target changed to %d%%\n",targetHumidity.getNewVal());
        }

        return(true);
    }

void loop() override {

        float temp = tempHumSensor.readTemperature();

        /* Update the temperature values in case they are out of range */
        if (temp < DHT_TEMPERATURE_DEFAULT_MIN_VAL) {
            temp = DHT_TEMPERATURE_DEFAULT_MIN_VAL;
        }

        if (temp > DHT_TEMPERATURE_DEFAULT_MAX_VAL) {
            temp = DHT_TEMPERATURE_DEFAULT_MAX_VAL;
        }

        /* If it's been more than DHT_POLLING_TIME seconds since last update, and temperature has changed */
        if (currentTemp.timeVal() > DHT_POLLING_TIME && fabs(currentTemp.getVal<float>() - temp) > 0.25) {
            currentTemp.setVal(temp);
            Serial.printf("Current Temperature is now %s.\n", temp2String(currentTemp.getNewVal<float>()).c_str());
        }

        switch(targetState.getVal()) {
            case E_THERMOSTAT_STATE_OFF:
                if (currentState.getVal() != E_THERMOSTAT_STATE_OFF) {
                    Serial.printf("Thermostat OFF\n");
                    currentState.setVal(0);
                    if (coolingDevice.getPowerState() == DAIKIN_POWER_ON) {
                        (void)powerOnOffDaikin(DAIKIN_POWER_OFF);
                    }
                }
                break;

            case E_THERMOSTAT_STATE_HEAT:
                if (currentTemp.getVal<float>() < targetTemp.getVal<float>() && currentState.getVal() != E_THERMOSTAT_STATE_HEAT) {
                    Serial.printf("Turning HEAT ON\n");
                    currentState.setVal(1);

                } else if (currentTemp.getVal<float>() >= targetTemp.getVal<float>() && currentState.getVal() == E_THERMOSTAT_STATE_HEAT) {
                    Serial.printf("Turning HEAT OFF\n");
                    currentState.setVal(0);

                } else if (currentState.getVal() == E_THERMOSTAT_STATE_COOL) {
                    Serial.printf("Turning COOL OFF\n");
                    if (coolingDevice.getPowerState() == DAIKIN_POWER_ON) {
                        (void)powerOnOffDaikin(DAIKIN_POWER_OFF);
                    }
                    currentState.setVal(0);
                }
                break;

            case E_THERMOSTAT_STATE_COOL:
                if (currentTemp.getVal<float>() > targetTemp.getVal<float>() && currentState.getVal() != E_THERMOSTAT_STATE_COOL) {
                    Serial.printf("Turning COOL ON\n");
                    if (coolingDevice.getPowerState() == DAIKIN_POWER_OFF) {
                        (void)powerOnOffDaikin(DAIKIN_POWER_ON);
                    }

                    (void)coolingDevice.setTemperature(E_MODE_COOL, targetTemp.getVal<float>());
                    currentState.setVal(2);

                } else if (currentTemp.getVal<float>() <= targetTemp.getVal<float>() && currentState.getVal() == E_THERMOSTAT_STATE_COOL) {
                    Serial.printf("Turning COOL OFF\n");
                    if (coolingDevice.getPowerState() == DAIKIN_POWER_ON) {
                        (void)powerOnOffDaikin(DAIKIN_POWER_OFF);
                    }
                    currentState.setVal(0);

                } else if (currentState.getVal() == E_THERMOSTAT_STATE_HEAT) {
                    Serial.printf("Turning HEAT OFF\n");
                    currentState.setVal(0);
                }
                break;

            case E_THERMOSTAT_STATE_AUTO:
                if (currentTemp.getVal<float>() < heatingThreshold.getVal<float>() && currentState.getVal() != E_THERMOSTAT_STATE_HEAT) {
                    Serial.printf("Turning HEAT ON\n");
                    currentState.setVal(1);

                } else if (currentTemp.getVal<float>() >= heatingThreshold.getVal<float>() && currentState.getVal() == E_THERMOSTAT_STATE_HEAT) {
                    Serial.printf("Turning HEAT OFF\n");
                    currentState.setVal(0);
                }

                if (currentTemp.getVal<float>() > coolingThreshold.getVal<float>() && currentState.getVal() != E_THERMOSTAT_STATE_COOL) {
                    Serial.printf("Turning COOL ON\n");
                    if (coolingDevice.getPowerState() == DAIKIN_POWER_OFF) {
                        (void)powerOnOffDaikin(DAIKIN_POWER_ON);
                    }
                    currentState.setVal(2);

                } else if (currentTemp.getVal<float>() <= coolingThreshold.getVal<float>() && currentState.getVal() == E_THERMOSTAT_STATE_COOL) {
                    Serial.printf("Turning COOL OFF\n");
                    if (coolingDevice.getPowerState() == DAIKIN_POWER_ON) {
                        (void)powerOnOffDaikin(DAIKIN_POWER_OFF);
                    }
                    currentState.setVal(0);
                }
                break;
        }
    }

private:
    t_httpErrorCodes powerOnOffDaikin(bool power) {
        t_httpErrorCodes daikinError;
        uint8_t retry = 0;

        do {
            daikinError = coolingDevice.powerOnOff(power);
            retry++;

            /* Wait 1 second before next retry */
            delay(1000);
        } while ((daikinError != E_REQUEST_SUCCESS) || (retry > DAIKIN_RETRY_MAX));

        if (daikinError == E_REQUEST_FAILURE) {
            Serial.println("Can not turn on/off Daikin");
        }

        return (daikinError);
    }

    String temp2String(float temp) {
        String t = displayUnits.getVal() ? String(round(temp * 1.8 + 32.0)) : String(temp);
        t += displayUnits.getVal() ? " F" : " C";
        return (t);
    }
};

#endif /* THERMOSTAT_H */
