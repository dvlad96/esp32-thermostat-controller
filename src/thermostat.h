#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include "HomeSpan.h"

/******** Typedef Defines ********/
typedef enum {
    E_THERMOSTAT_STATE_OFF      = 0,
    E_THERMOSTAT_STATE_HEAT     = 1,
    E_THERMOSTAT_STATE_COOL     = 2,
    E_THERMOSTAT_STATE_AUTO     = 3
} t_thermostatStates;


struct HS_Thermostat : Service::Thermostat {

    // Create characteristics, set initial values, and set storage in NVS to true
    Characteristic::CurrentHeatingCoolingState currentState{0, true};
    Characteristic::TargetHeatingCoolingState targetState{0, true};
    Characteristic::CurrentTemperature currentTemp{22, true};
    Characteristic::TargetTemperature targetTemp{22, true};
    Characteristic::CurrentRelativeHumidity currentHumidity{50, true};
    Characteristic::TargetRelativeHumidity targetHumidity{50, true};
    Characteristic::HeatingThresholdTemperature heatingThreshold{22, true};
    Characteristic::CoolingThresholdTemperature coolingThreshold{22, true};
    Characteristic::TemperatureDisplayUnits displayUnits{0, true}; // this is for changing the display on the actual thermostat (if any), NOT in the Home App

    HS_Thermostat() : Service::Thermostat() {
        Serial.printf("\n*** Creating HomeSpan Thermostat***\n");
        new SpanUserCommand('t', "<temp> - set the temperature, where temp is in F or C depending on configuration", setTemp, this);
    }

    boolean update() override {
        if (targetState.updated()) {
            switch (targetState.getNewVal()) {
                case E_THERMOSTAT_STATE_OFF:
                    Serial.printf("Thermostat turning OFF\n");
                    break;
                case E_THERMOSTAT_STATE_HEAT:
                    Serial.printf("Thermostat set to HEAT at %s\n", temp2String(targetTemp.getVal<float>()).c_str());
                    break;
                case E_THERMOSTAT_STATE_COOL:
                    Serial.printf("Thermostat set to COOL at %s\n", temp2String(targetTemp.getVal<float>()).c_str());
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
                    currentState.setVal(2);
                } else if (currentTemp.getVal<float>() <= targetTemp.getVal<float>() && currentState.getVal() == 2) {
                    Serial.printf("Turning COOL OFF\n");
                    currentState.setVal(0);
                } else if (currentState.getVal() == 1) {
                    Serial.printf("Turning HEAT OFF\n");
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

    static void setTemp(const char *buf, void *arg) {
        HS_Thermostat *thermostat = (HS_Thermostat *)arg;

        float temp = atof(buf + 1);
        float tempC = temp;

        if (thermostat->displayUnits.getVal()) {
            tempC = (temp - 32.0) / 1.8;
        }

        if (tempC < 10.0 || tempC > 38.0) {
            Serial.printf("usage: @t <temp>, where temp is in range of 10C (50F) through 38C (100F)\n\n");
            return;
        }

        Serial.printf("Current temperature is now %.1f %c\n", temp, thermostat->displayUnits.getVal() ? 'F' : 'C');
        thermostat->currentTemp.setVal(tempC);
    }
};

#endif /* THERMOSTAT_H */
