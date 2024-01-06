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

#define THERMOSTAT_HYSTERESIS                       (0.5)

#define TEMPERATURE_ALPHA                           (0.15)

#define THERMOSTAT_STATUS_UPDATE_POLLING_TIME       (10000U)

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

typedef enum  {
    E_IDLE = 0,
    E_HEATING = 1,
    E_COOLING = 2 // we disable this state since we don't have an AC (in essence this is just a bool)
}t_currentHeaterStates;

enum TemperatureDisplayUnits {
  CELSIUS = 0,
  FAHRENHEIT = 1 // we disable this state since we are not heathens
};

/************************************************
 *  Class definition
 ***********************************************/
class CurrentHeaterStatus: public Characteristic::CurrentHeatingCoolingState {
private:
    /** @brief Heating relay Object */
    esp01sRelay heatingDevice {"192.168.1.100", 80};

public:
    /** @brief Constructor */
    CurrentHeaterStatus(): Characteristic::CurrentHeatingCoolingState(E_THERMOSTAT_STATE_OFF) {
        /* Initialize the Heating Device */
        heatingDevice.espNowCommunicationSetup();
    }

    template <typename T>
    void setVal(T value, bool notify = true) {
        Serial.printf("Sending %d value to the heater\n", value);
        (void)heatingDevice.sendRelayCommand(value);
        Characteristic::CurrentHeatingCoolingState::setVal(value, notify);
    }
};

struct HS_Thermostat : Service::Thermostat {
private:
    /** @brief Characteristic objects */
    CurrentHeaterStatus * currentState;
    SpanCharacteristic * targetState;
    SpanCharacteristic * currentTemp;
    SpanCharacteristic * targetTemp;
    SpanCharacteristic * currentHumidity;
    SpanCharacteristic * targetHumidity;
    SpanCharacteristic * heatingThreshold;
    SpanCharacteristic * coolingThreshold;
    SpanCharacteristic * displayUnits;;

    /** @brief DHT Temperature Sensor Object */
    DHT tempHumSensor {DHT_PIN, DHT_TYPE};

    /** @brief Average temperature */
    float averageTemp;

    /** @brief Flag to track if user manually updated the thermostat */
    bool wasUpdated;

    /** @brief Variable used to track the millis passed since last Temperature update */
    unsigned long lastUpdateTemperature;

    /** @brief Variable used to track the millis passed since last temperature readout */
    unsigned long lastSenseTemperature;

    /** @brief Variable used to track the millis passed since last thermostat overall update */
    unsigned long lastUpdateState;

public:
    /** @brief Constructor */
    HS_Thermostat() : Service::Thermostat() {

        /* Initialize DHT sensor */
        tempHumSensor.begin();

        /* Get an initial temperature read */
        averageTemp = tempHumSensor.readTemperature();

        /* Initialize the Characteristics */
        currentState = new CurrentHeaterStatus();
        targetState =  new Characteristic::TargetHeatingCoolingState(E_THERMOSTAT_STATE_OFF, true);

        currentTemp = new Characteristic::CurrentTemperature(averageTemp);
        targetTemp = new Characteristic::TargetTemperature(THERMOSTAT_DEFAULT_TARGET_TEMPERATURE);
        currentHumidity = new Characteristic::CurrentRelativeHumidity(THERMOSTAT_DEFAULT_TARGET_HUMIDITY);
        targetHumidity = new Characteristic::TargetRelativeHumidity(THERMOSTAT_DEFAULT_TARGET_HUMIDITY);

        heatingThreshold = new Characteristic::CoolingThresholdTemperature(24, true);
        coolingThreshold = new Characteristic::HeatingThresholdTemperature(22, true);

        displayUnits =  new Characteristic::TemperatureDisplayUnits(CELSIUS);

        /* setup the valid values for characteristics */
        currentState->setValidValues(2, E_IDLE, E_HEATING);
        targetState->setValidValues(3, E_THERMOSTAT_STATE_OFF, E_THERMOSTAT_STATE_HEAT, E_THERMOSTAT_STATE_AUTO);
        displayUnits->setValidValues(1, CELSIUS);
        displayUnits->removePerms(PW);

        /* set the ranges and step values of temperatures */
        targetTemp->setRange(10, 38, 0.5);
        currentTemp->setRange(0, 100, 0.1);
        coolingThreshold->setRange(0, 100, 0.5);
        heatingThreshold->setRange(0, 100, 0.5);

        // setup the state of the thermostat
        lastUpdateTemperature = millis();
        lastSenseTemperature = millis();
        lastUpdateState = millis();
        wasUpdated = false;

        /* In case of a sudden reset, turn of the Heating */
        currentState->setVal(RELAY_COMMAND_OFF);
        targetState->setVal((int)E_IDLE);
    }

    /** @brief Update function override */
    boolean update() override {

        /* Check if the user updated any parameter */
        wasUpdated = targetState->updated()      ||
                     targetTemp->updated()       ||
                     coolingThreshold->updated() ||
                     heatingThreshold->updated();

        return(true);
    }

    /** @brief Loop function override */
    void loop() override {

        /* sense temperature every given duration */
        if ((millis() - lastSenseTemperature) > TEMPERATURE_SENSOR_POLLING_TIME) {
            updateTempReading();
            lastSenseTemperature = millis();
        }

        /* update current temperature every given duration but only when when you have enough readings */
        if ((millis() - lastUpdateTemperature) > THERMOSTAT_STATUS_UPDATE_POLLING_TIME) {
            updateCurrentTemp();
            lastUpdateTemperature = millis();
        }

        /* update state every given duration */
        if ((millis() - lastUpdateState) > THERMOSTAT_STATUS_UPDATE_POLLING_TIME || wasUpdated) {
            updateState();
            wasUpdated = false;
            lastUpdateState = millis();
        }
    }

    /* update the state of the system given parameters */
    void updateState() {
        /* get the current state of the system */
        bool toggle = false;
        bool heaterState = currentState->getVal();
        Serial.printf("Heater state = %d\n", heaterState);
        float targetTemp = this->targetTemp->getVal<float>();
        float maxTemp = heatingThreshold->getVal<float>();
        float minTemp = coolingThreshold->getVal<float>();

        /* decide whether to changed the state of the heater based on currently set config */
        switch (targetState->getVal()) {
            case E_THERMOSTAT_STATE_OFF:
                toggle = (bool)E_ESP01S_RELAY_OPEN;
                break;
            case E_THERMOSTAT_STATE_HEAT:
                toggle = toggleManualHeaterState(averageTemp, targetTemp, heaterState);
                break;
            case E_THERMOSTAT_STATE_AUTO:
            default:
                toggle = toggleAutoHeaterState(averageTemp, minTemp, maxTemp, heaterState);
        }

        if (toggle != heaterState) {
            currentState->setVal(toggle);
        }
    }

    /* accumulate a new temperature reading using exponential averaging */
    void updateTempReading() {
        /* read the current temperature */
        float reading = tempHumSensor.readTemperature();

        /* validate for correct seaming reading and accumulate if so */
        if ((DHT_TEMPERATURE_DEFAULT_MIN_VAL <= reading) &&
            (reading <= DHT_TEMPERATURE_DEFAULT_MAX_VAL)) {
            averageTemp *= TEMPERATURE_ALPHA;
            averageTemp += (1 - TEMPERATURE_ALPHA) * reading;
        }
    }

    /* update the current temperature from accumulated readings */
    void updateCurrentTemp() {
        currentTemp->setVal<float>(averageTemp);
    }

    /* Manual mode Heater Control */
    bool toggleManualHeaterState(float currentTemp, float targetTemp, bool heaterState) {

        bool toggle = heaterState;

        if (currentTemp <= (targetTemp - THERMOSTAT_HYSTERESIS) && heaterState == false) {
            toggle = true;
        } else if (currentTemp >= (targetTemp + THERMOSTAT_HYSTERESIS) && heaterState == true) {
            toggle = false;
        } else {
            toggle = heaterState;
        }

        return (toggle);
    }

    /* Auto mode Heater Control */
    bool toggleAutoHeaterState(float currentTemp, float minTemp, float maxTemp, bool heaterState) {

        bool toggle = heaterState;

        if (currentTemp <= (minTemp - THERMOSTAT_HYSTERESIS) && heaterState == false) {
            toggle = true;
        } else if (currentTemp >= (maxTemp + THERMOSTAT_HYSTERESIS) && heaterState == true) {
            toggle = false;
        } else {
            toggle = heaterState;
        }
      return (toggle);
    }

};

#endif /* THERMOSTAT_H */
