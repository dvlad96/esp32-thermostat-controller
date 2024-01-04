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
/** @brief Default Target Temperature */
#define THERMOSTAT_DEFAULT_TARGET_TEMPERATURE       (22U)

/** @brief Default Target Humidity */
#define THERMOSTAT_DEFAULT_TARGET_HUMIDITY          (50U)

/** @brief Thermostat retry timeout */
#define THERMOSTAT_RETRY_TIME                       (1000U)

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
private:
    /** @brief Characteristic objects */
    Characteristic::CurrentHeatingCoolingState currentState{0U, true};
    Characteristic::TargetHeatingCoolingState targetState{0U, true};
    Characteristic::CurrentTemperature currentTemp{THERMOSTAT_DEFAULT_TARGET_TEMPERATURE, true};
    Characteristic::TargetTemperature targetTemp{THERMOSTAT_DEFAULT_TARGET_TEMPERATURE, true};
    Characteristic::CurrentRelativeHumidity currentHumidity{THERMOSTAT_DEFAULT_TARGET_TEMPERATURE, true};
    Characteristic::TargetRelativeHumidity targetHumidity{THERMOSTAT_DEFAULT_TARGET_TEMPERATURE, true};
    Characteristic::HeatingThresholdTemperature heatingThreshold{THERMOSTAT_DEFAULT_TARGET_TEMPERATURE, true};
    Characteristic::CoolingThresholdTemperature coolingThreshold{THERMOSTAT_DEFAULT_TARGET_TEMPERATURE, true};
    Characteristic::TemperatureDisplayUnits displayUnits{0U, true};

    /** @brief DHT Temperature Sensor Object */
    DHT tempHumSensor {DHT_PIN, DHT_TYPE};

    /** @brief Daikin AC Object */
    daikin coolingDevice {(char *)DAIKIN_IP_ADDRESS, DAIKIN_PORT_ID};

    /** @brief Heating relay Object */
    heatingRelay heatingDevice;

    /**
     * @brief Power ON/OFF Daikin private method
     * @details
     *  This method is responsible to send ON/OFF commands to the Daikin AC
     *
     * @param power DAIKIN_POWER_ON or DAIKIN_POWER_OFF
     * @return t_httpErrorCodes
     */
    t_httpErrorCodes powerOnOffDaikin(bool power) {

        t_httpErrorCodes daikinError;
        uint8_t retry = 0U;

        do {
            daikinError = coolingDevice.powerOnOff(power);
            retry++;

            /* Wait THERMOSTAT_RETRY_TIME second before next retry */
            delay(THERMOSTAT_RETRY_TIME);
        } while ((daikinError != E_REQUEST_SUCCESS) || (retry < DAIKIN_RETRY_MAX));

        if (daikinError == E_REQUEST_FAILURE) {
            Serial.println("Can not turn on/off Daikin");
        }

        return (daikinError);
    }

    /**
     * @brief Temperature to String private method
     * @details
     *  This method is responsible to convert the input @param temp to a String
     *
     * @param temp  Temperature
     * @return String
     */
    String temp2String(const float temp) {
        String t = displayUnits.getVal() ? String(round(temp * 1.8 + 32.0)) : String(temp);
        t += displayUnits.getVal() ? " F" : " C";
        return (t);
    }

public:
    /** @brief Constructor */
    HS_Thermostat() : Service::Thermostat() {

        /* Initialize DHT sensor */
        tempHumSensor.begin();

        /* Initialize the Heating Device */
        heatingDevice.espNowCommunicationSetup();

        /* Set the initial values */
        currentTemp.setVal(tempHumSensor.readTemperature());
        currentTemp.setRange(DHT_TEMPERATURE_DEFAULT_MIN_VAL, DHT_TEMPERATURE_DEFAULT_MAX_VAL);
        targetTemp.setVal(THERMOSTAT_DEFAULT_TARGET_TEMPERATURE);
        targetTemp.setRange(DHT_TEMPERATURE_DEFAULT_MIN_VAL, DHT_TEMPERATURE_DEFAULT_MAX_VAL);

        currentHumidity.setVal(tempHumSensor.readHumidity());
        currentHumidity.setRange(DHT_HUMIDITY_DEFAULT_MIN_RANGE, DHT_HUMIDITY_DEFAULT_MAX_RANGE);
        targetHumidity.setVal(THERMOSTAT_DEFAULT_TARGET_HUMIDITY);
        currentHumidity.setRange(DHT_HUMIDITY_DEFAULT_MIN_RANGE, DHT_HUMIDITY_DEFAULT_MAX_RANGE);

        heatingThreshold.setRange(DHT_TEMPERATURE_DEFAULT_MIN_VAL, DHT_TEMPERATURE_DEFAULT_MAX_VAL);
        coolingThreshold.setRange(DHT_TEMPERATURE_DEFAULT_MIN_VAL, DHT_TEMPERATURE_DEFAULT_MAX_VAL);
    }

    /** @brief Update function override */
    boolean update() override {
        if (targetState.updated()) {
            switch(targetState.getNewVal()) {
                case E_THERMOSTAT_STATE_OFF:
                    Serial.printf("Thermostat turning OFF\n");

                    /* Turn off the AC */
                    if (coolingDevice.getPowerState() == DAIKIN_POWER_ON) {
                        (void)powerOnOffDaikin(DAIKIN_POWER_OFF);
                    } else {
                        /* AC already turned OFF */
                    }

                    /* Turn off the Heating Relay */
                    (void)heatingDevice.sendRelayCommand(RELAY_COMMAND_OFF);

                    break;

                case E_THERMOSTAT_STATE_HEAT:
                    Serial.printf("Thermostat set to HEAT at %s\n",
                                   temp2String(targetTemp.getVal<float>()).c_str());

                    if (coolingDevice.getPowerState() == DAIKIN_POWER_ON) {
                        (void)powerOnOffDaikin(DAIKIN_POWER_OFF);
                    } else {
                        /* AC already turned OFF */
                    }

                    /* Turn on the Heating Relay */
                    (void)heatingDevice.sendRelayCommand(RELAY_COMMAND_ON);

                    break;

                case E_THERMOSTAT_STATE_COOL:
                    Serial.printf("Thermostat set to COOL at %s\n",
                                  temp2String(targetTemp.getVal<float>()).c_str());

                    if (coolingDevice.getPowerState() == DAIKIN_POWER_OFF) {
                        (void)powerOnOffDaikin(DAIKIN_POWER_ON);
                    } else {
                        /* AC already turned OFF */
                    }

                    /* Turn off the Heating Relay */
                    (void)heatingDevice.sendRelayCommand(RELAY_COMMAND_OFF);

                    break;

                case E_THERMOSTAT_STATE_AUTO:
                    Serial.printf("Thermostat set to AUTO from %s to %s\n",
                                  temp2String(heatingThreshold.getVal<float>()).c_str(),temp2String(coolingThreshold.getVal<float>()).c_str());

                    /* For now, turn off everything */
                    if (coolingDevice.getPowerState() == DAIKIN_POWER_OFF) {
                        (void)powerOnOffDaikin(DAIKIN_POWER_ON);
                    } else {
                        /* AC already turned OFF */
                    }
                    (void)heatingDevice.sendRelayCommand(RELAY_COMMAND_OFF);

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

    /** @brief Loop function override */
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
                    (void)heatingDevice.sendRelayCommand(RELAY_COMMAND_OFF);
                }
                break;

            case E_THERMOSTAT_STATE_HEAT:
                if (currentTemp.getVal<float>() < targetTemp.getVal<float>() && currentState.getVal() != E_THERMOSTAT_STATE_HEAT) {
                    Serial.printf("Turning HEAT ON\n");
                    currentState.setVal(1);
                    (void)heatingDevice.sendRelayCommand(RELAY_COMMAND_ON);

                } else if (currentTemp.getVal<float>() >= targetTemp.getVal<float>() && currentState.getVal() == E_THERMOSTAT_STATE_HEAT) {
                    Serial.printf("Turning HEAT OFF\n");
                    currentState.setVal(0);
                    (void)heatingDevice.sendRelayCommand(RELAY_COMMAND_OFF);

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
                    (void)heatingDevice.sendRelayCommand(RELAY_COMMAND_OFF);
                }
                break;

            case E_THERMOSTAT_STATE_AUTO:
                if (currentTemp.getVal<float>() < heatingThreshold.getVal<float>() && currentState.getVal() != E_THERMOSTAT_STATE_HEAT) {
                    Serial.printf("Turning HEAT ON\n");
                    currentState.setVal(1);
                    (void)heatingDevice.sendRelayCommand(RELAY_COMMAND_ON);

                } else if (currentTemp.getVal<float>() >= heatingThreshold.getVal<float>() && currentState.getVal() == E_THERMOSTAT_STATE_HEAT) {
                    Serial.printf("Turning HEAT OFF\n");
                    currentState.setVal(0);
                    (void)heatingDevice.sendRelayCommand(RELAY_COMMAND_OFF);
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
};

#endif /* THERMOSTAT_H */
