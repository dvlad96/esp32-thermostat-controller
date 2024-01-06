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
/** @brief Default AC min temperature */
#define AC_DEFAULT_MIN_TEMP                 (10U)

/** @brief Default AC max temperature */
#define AC_DEFAULT_MAX_TEMP                 (30U)

/************************************************
 *  Typedef definition
 ***********************************************/
/** @brief This enum represents all the possible states of a Home Kit AC */
typedef enum {
    E_AC_STATE_OFF      = 0U,           /**< AC OFF */
    E_AC_STATE_HEAT     = 1U,           /**< AC set on HEAT mode */
    E_AC_STATE_COOL     = 2U,           /**< AC set on COOL mode */
    E_AC_STATE_AUTO     = 3U            /**< AC set on AUTO mode */
} t_acState;

/************************************************
 *  Class definition
 ***********************************************/
struct HS_AirConditioner : Service::HeaterCooler {
private:
    /** @brief Characteristic objects */
    SpanCharacteristic * active;
    SpanCharacteristic * currentTemperature;
    SpanCharacteristic * currentState;
    SpanCharacteristic * targetState;
    SpanCharacteristic * targetCoolingTemperature;
    SpanCharacteristic * targetHeatingTemperature;
    SpanCharacteristic * temperatureDisplayUnits;
    SpanCharacteristic * swingMode;
    SpanCharacteristic * rotationSpeed;

    /** @brief Daikin object */
    daikin ac {(char *)DAIKIN_IP_ADDRESS, DAIKIN_PORT_ID};

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

    /**
     * @brief Temperature to String private method
     * @details
     *  This method is responsible to convert the input @param temp to a String
     *
     * @param temp  Temperature
     * @return String
     */
    String temp2String(const float temp) {
        String t = temperatureDisplayUnits->getVal() ? String(round(temp * 1.8 + 32.0)) : String(temp);
        t += temperatureDisplayUnits->getVal() ? " F" : " C";
        return (t);
    }

public:
    /** @brief Constructor */
    HS_AirConditioner() : Service::HeaterCooler() {
        Serial.printf("\n*** Creating HomeSpan HeaterCooler***\n");

        /* Initialize the Characteristics objects */
        active = new Characteristic::Active();                                          /* Active or not */
        currentState = new Characteristic::CurrentHeaterCoolerState();                  /* Current status, heating if below threshold, cooling if above, waiting... */
        targetState = new Characteristic::TargetHeaterCoolerState(1);                   /* Set state, heat, cool, auto, off... */
        currentTemperature = new Characteristic::CurrentTemperature(20);                /* Current temp */
        targetCoolingTemperature = new Characteristic::CoolingThresholdTemperature(20); /* Cooling threshold */
        targetHeatingTemperature = new Characteristic::HeatingThresholdTemperature(20); /* Heating threshold */
        temperatureDisplayUnits = new Characteristic::TemperatureDisplayUnits(0);       /* Default to Celsius */
        swingMode = new Characteristic::SwingMode();                                    /* Swing mode */
        rotationSpeed = new Characteristic::RotationSpeed();                            /* Rotation speed */

        currentTemperature->setRange(AC_DEFAULT_MIN_TEMP, AC_DEFAULT_MAX_TEMP);
        targetCoolingTemperature->setRange(AC_DEFAULT_MIN_TEMP, AC_DEFAULT_MAX_TEMP);
        targetHeatingTemperature-> setRange(AC_DEFAULT_MIN_TEMP, AC_DEFAULT_MAX_TEMP);
    }

    /** @brief Update function override */
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

    /** @brief Loop function override */
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

        /* Once every DAIKIN_AC_TEMPERATURE_REFRESH, check the sensor temperature */
        if (currentTemperature->timeVal() > DAIKIN_AC_TEMPERATURE_REFRESH) {
            float readoutTemperature = 0;
            (void)ac.getCurrentTemperature(&readoutTemperature);

            currentTemperature->setVal(readoutTemperature);
            //Serial.printf("AC: New readout temperature = %.2f \n", readoutTemperature);
        }
    }
};

#endif /* AIR_CONDITIONER_H */
