#ifndef AIR_CONDITIONER_H
#define AIR_CONDITIONER_H

/************************************************
 *  Includes
 ***********************************************/
#include "HomeSpan.h"

/* Local files */
#include "devices/daikin.h"
#include "devices/deviceInfo.h"

/************************************************
 *  Defines / Macros
 ***********************************************/

/************************************************
 *  Typedef definition
 ***********************************************/
/** @brief This enum represents all the possible states of a Home Kit AC */
typedef enum {
    E_AC_STATE_OFF      = 0U,           /**< AC OFF */
    E_AC_STATE_HEAT     = 1U,           /**< AC set on HEAT mode */
    E_AC_STATE_COOL     = 2U,           /**< AC set on COOL mode */
    E_AC_STATE_AUTO     = 3U            /**< AC set on AUTO mode */
} t_acStates;

/************************************************
 *  Class definition
 ***********************************************/
struct HS_AirConditioner : Service::HeaterCooler {
public:
    /* Characteristic objects */
    SpanCharacteristic *active;
    SpanCharacteristic *currentTemperature;
    SpanCharacteristic *currentState;
    SpanCharacteristic *targetState;
    SpanCharacteristic *targetCoolingTemperature;
    SpanCharacteristic *targetHeatingTemperature;
    SpanCharacteristic *temperatureDisplayUnits;
    SpanCharacteristic *swingMode;
    SpanCharacteristic *rotationSpeed;

    daikin ac {(char *)DAIKIN_IP_ADDRESS, DAIKIN_PORT_ID};

    HS_AirConditioner() : Service::HeaterCooler() {
        Serial.printf("\n*** Creating HomeSpan HeaterCooler***\n");

        active = new Characteristic::Active(); // active or not
        currentState = new Characteristic::CurrentHeaterCoolerState(); //current status, heating if below threshold, cooling if above, waiting...
        targetState = new Characteristic::TargetHeaterCoolerState(1);  //Set state, heat, cool, auto, off...
        currentTemperature = new Characteristic::CurrentTemperature(20); //current temp
        targetCoolingTemperature = new Characteristic::CoolingThresholdTemperature(20); // cooling threshold
        targetHeatingTemperature = new Characteristic::HeatingThresholdTemperature(20); // heating threshold
        temperatureDisplayUnits = new Characteristic::TemperatureDisplayUnits(0); // Celsius please.
        swingMode = new Characteristic::SwingMode();
        rotationSpeed = new Characteristic::RotationSpeed();

        currentTemperature->setRange(10, 30);
        targetCoolingTemperature->setRange(10,30);
        targetHeatingTemperature-> setRange(10,30);
    }

    boolean update() override {
        int fanSpeed;
        bool fanSwing;
        t_httpErrorCodes daikinError;

        /* Check if the Target State has been updated */
        if (targetState->updated()) {
            switch (targetState->getNewVal()) {
                case E_AC_STATE_OFF:
                    Serial.printf("Thermostat turning OFF\n");
                    daikinError = powerOnOffDaikin(DAIKIN_POWER_OFF);
                    break;
                case E_AC_STATE_HEAT:
                    Serial.printf("Thermostat set to HEAT at %s\n", temp2String(targetHeatingTemperature->getVal<float>()).c_str());
                    daikinError = ac.setTemperature(E_MODE_HEAT, targetHeatingTemperature->getVal<float>());
                    break;
                case E_AC_STATE_COOL:
                    Serial.printf("Thermostat set to COOL at %s\n", temp2String(targetCoolingTemperature->getVal<float>()).c_str());
                    daikinError = ac.setTemperature(E_MODE_COOL, targetCoolingTemperature->getVal<float>());
                    break;
                case E_AC_STATE_AUTO:
                    daikinError = powerOnOffDaikin(DAIKIN_POWER_OFF);
                    Serial.printf("Thermostat set to AUTO from %s to %s\n", temp2String(targetHeatingTemperature->getVal<float>()).c_str(), temp2String(targetCoolingTemperature->getVal<float>()).c_str());
                    break;
            }
        }

        if (targetCoolingTemperature->updated()) {
            (void)ac.setTemperature(E_MODE_COOL, targetCoolingTemperature->getNewVal<float>());
        }

        if (targetHeatingTemperature->updated()) {
            (void)ac.setTemperature(E_MODE_HEAT, targetHeatingTemperature->getNewVal<float>());
        }

        /* Check if the Fan Speed has been updated */
        if (rotationSpeed->updated()) {
            fanSpeed = rotationSpeed->getNewVal();
            Serial.printf("New FAN SPEED = %d", fanSpeed);
            ac.setFanSpeed(M_SET_FAN_SPEED(fanSpeed));
        }

        /* Check if the Fan Swing has been updated */
        if (swingMode->updated()) {
            fanSwing = swingMode->getNewVal<bool>();
            Serial.printf("New FAN SWING = %d", fanSwing);
            ac.setFanSwingMode(fanSwing == false ? E_STOP : E_ALL);
        }

        return (true);
    }

    void loop() override {
        switch (currentState->getVal()) {
            case E_AC_STATE_HEAT:
                /* Check if the target temperature has changed */
                if (targetHeatingTemperature->updated()) {
                    ac.setTemperature(E_MODE_HEAT, targetHeatingTemperature->getNewVal<float>());
                }
                break;
            case E_AC_STATE_COOL:
                /* Check if the target temperature has changed */
                if (targetHeatingTemperature->updated()) {
                    ac.setTemperature(E_MODE_COOL, targetCoolingTemperature->getNewVal<float>());
                }
            case E_AC_STATE_OFF:
            E_AC_STATE_AUTO:
            default:
                break;
        }

        /* Once every 5 minutes, check the sensor temperature */
        if (currentTemperature->timeVal() > DAIKIN_AC_TEMPERATURE_REFRESH) {
            float readoutTemperature = 0;
            (void)ac.getCurrentTemperature(&readoutTemperature);

            currentTemperature->setVal(readoutTemperature);
            Serial.printf("AC: New readout temperature = %.2f \n", readoutTemperature);
        }
    }

private:
    t_httpErrorCodes powerOnOffDaikin(bool power) {
        t_httpErrorCodes daikinError;
        uint8_t retry = 0;

        do {
            daikinError = ac.powerOnOff(power);
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
        String t = temperatureDisplayUnits->getVal() ? String(round(temp * 1.8 + 32.0)) : String(temp);
        t += temperatureDisplayUnits->getVal() ? " F" : " C";
        return (t);
    }

};

#endif /* AIR_CONDITIONER_H */
