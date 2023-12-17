#ifndef AIR_CONDITIONER_H
#define AIR_CONDITIONER_H

#include "HomeSpan.h"
#include "devices/daikin.h"

/******** Typedef Defines ********/
typedef enum {
    E_AC_STATE_OFF      = 0,
    E_AC_STATE_HEAT     = 1,
    E_AC_STATE_COOL     = 2,
    E_AC_STATE_AUTO     = 3
} t_acStates;

struct HS_AirConditioner : Service::HeaterCooler, daikin {

    SpanCharacteristic *active;
    SpanCharacteristic *currentTemperature;
    SpanCharacteristic *currentState;
    SpanCharacteristic *targetState;
    SpanCharacteristic *targetCoolingTemperature;
    SpanCharacteristic *targetHeatingTemperature;
    SpanCharacteristic *temperatureDisplayUnits;
    SpanCharacteristic *swingMode;
    SpanCharacteristic *rotationSpeed;

    HS_AirConditioner(char * daikinIpAddress, const int daikinPortId) : Service::HeaterCooler(), daikin(daikinIpAddress, daikinPortId) {
        Serial.printf("\n*** Creating HomeSpan HeaterCooler***\n");
        //new SpanUserCommand('t', "<temp> - set the temperature, where temp is in F or C depending on configuration", setTemp, this);

        active = new Characteristic::Active(); // active or not
        currentState = new Characteristic::CurrentHeaterCoolerState(); //current status, heating if below threshold, cooling if above, waiting...
        targetState = new Characteristic::TargetHeaterCoolerState(1);  //Set state, heat, cool, auto, off...
        currentTemperature = new Characteristic::CurrentTemperature(20); //current temp
        targetCoolingTemperature = new Characteristic::CoolingThresholdTemperature(20); // cooling threshold
        targetHeatingTemperature = new Characteristic::HeatingThresholdTemperature(20); // heating threshold
        temperatureDisplayUnits = new Characteristic::TemperatureDisplayUnits(0); // Celsius please.
        swingMode = new Characteristic::SwingMode();
        rotationSpeed = new Characteristic::RotationSpeed();

        currentTemperature->setRange(-10, 100);
        targetCoolingTemperature->setRange(18,30);
        targetHeatingTemperature-> setRange(18,28);
    }

    boolean update() override {
        int newState;
        int isActive;
        t_httpErrorCodes daikinError;

        if(active->updated()) {
            isActive = active->getNewVal();

            if (isActive == 0) {
                daikinError = this->powerOnOffDaikin(DAIKIN_POWER_OFF);
            } else {
                daikinError = this->powerOnOffDaikin(DAIKIN_POWER_ON);
            }
        }

        if (targetState->updated()) {
            newState = targetState->getNewVal();

            switch (newState) {
                case E_AC_STATE_OFF:
                    Serial.printf("Thermostat turning OFF\n");
                    daikinError = this->powerOnOffDaikin(DAIKIN_POWER_OFF);
                    break;
                case E_AC_STATE_HEAT:
                    Serial.printf("Thermostat set to HEAT at %s\n", temp2String(targetHeatingTemperature->getVal<float>()).c_str());
                    daikinError = this->setTemperature(E_MODE_HEAT, targetHeatingTemperature->getVal<float>());
                    break;
                case E_AC_STATE_COOL:
                    Serial.printf("Thermostat set to COOL at %s\n", temp2String(targetCoolingTemperature->getVal<float>()).c_str());
                    daikinError = this->setTemperature(E_MODE_COOL, targetCoolingTemperature->getVal<float>());
                    break;
                case E_AC_STATE_AUTO:
                    daikinError = this->powerOnOffDaikin(DAIKIN_POWER_OFF);
                    Serial.printf("Thermostat set to AUTO from %s to %s\n", temp2String(targetHeatingTemperature->getVal<float>()).c_str(), temp2String(targetCoolingTemperature->getVal<float>()).c_str());
                    break;
            }
        }

        return (true);
    }

    void loop() override {

    }

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

    String temp2String(float temp) {
        String t = temperatureDisplayUnits->getVal() ? String(round(temp * 1.8 + 32.0)) : String(temp);
        t += temperatureDisplayUnits->getVal() ? " F" : " C";
        return (t);
    }

};

#endif /* AIR_CONDITIONER_H */
