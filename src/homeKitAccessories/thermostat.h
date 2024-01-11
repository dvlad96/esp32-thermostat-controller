#ifndef THERMOSTAT_H
#define THERMOSTAT_H

/************************************************
 *  Includes
 ***********************************************/
#include "HomeSpan.h"

/* Local files */
#include "devices/esp01sRelay.h"
#include "devices/adafruitAht20.h"
#include "devices/deviceInfo.h"

/************************************************
 *  Defines / Macros
 ***********************************************/
/** @brief Default Target Humidity */
#define THERMOSTAT_DEFAULT_TARGET_HUMIDITY          (50U)

/** @brief Temperature hysteresis */
#define THERMOSTAT_AUTO_HYSTERESIS                  (0.1)
#define THERMOSTAT_MANUAL_HYSTERESIS                (0.5)

/** @brief Alpha for exponential average */
#define TEMPERATURE_ALPHA                           (0.75)

/** @brief Time in MS in between characteristics update */
#define THERMOSTAT_STATUS_UPDATE_POLLING_TIME       (30 * 1000U)

/************************************************
 *  Typedef definition
 ***********************************************/
/** @brief This enum represents all the possible states of a Home Kit Thermostat */
typedef enum {
    E_THERMOSTAT_STATE_OFF  = 0U,       /**< Thermostat OFF */
    E_THERMOSTAT_STATE_HEAT = 1U,       /**< Thermostat set on HEAT mode */
    E_THERMOSTAT_STATE_COOL = 2U,       /**< Thermostat set on COOL mode */
    E_THERMOSTAT_STATE_AUTO = 3U        /**< Thermostat set on AUTO mode */
} t_thermostatStates;

/** @brief This enum represents all the display options of the Home Kit Thermostat */
enum TemperatureDisplayUnits {
  E_CELSIUS    = 0U,                    /**< Display in Celsius */
  E_FAHRENHEIT = 1U                     /**< Display in Fahrenheit */
};

/************************************************
 *  Class definition
 ***********************************************/
class CurrentHeaterStatus: public Characteristic::CurrentHeatingCoolingState {
private:
    /** @brief Heating relay Object */
    Esp01sRelay heatingDevice {THERMOSTAT_RELAY_IP_ADDRESS, THERMOSTAT_RELAY_PORT_ID};

public:
    /** @brief Constructor */
    CurrentHeaterStatus(): Characteristic::CurrentHeatingCoolingState(E_THERMOSTAT_STATE_OFF) {}


    template <typename T>
    void setVal(T value, bool notify = true) {
        (void)heatingDevice.sendEsp01sRelayCommand((t_esp01sRelayState)value);
        Characteristic::CurrentHeatingCoolingState::setVal(value, notify);
    }

    t_esp01sRelayState getHeaterState(void) {
        t_esp01sRelayState state;
        if (heatingDevice.getEsp01sRelayState(&state) != E_REQUEST_SUCCESS) {
            state = E_ESP01S_RELAY_OPEN;
        }
        return (state);
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
    TempHumSensor tempHumSensor;

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
        (void)tempHumSensor.initializeSensor();

        /* Get an initial temperature read */
        averageTemp = tempHumSensor.getCurrentTemperature();

        /* Initialize the Characteristics */
        currentState = new CurrentHeaterStatus();
        targetState =  new Characteristic::TargetHeatingCoolingState(E_THERMOSTAT_STATE_OFF, true);

        currentTemp = new Characteristic::CurrentTemperature(averageTemp);
        targetTemp = new Characteristic::TargetTemperature(TEMPERATURE_INITIAL_VALUE);
        currentHumidity = new Characteristic::CurrentRelativeHumidity(THERMOSTAT_DEFAULT_TARGET_HUMIDITY);
        targetHumidity = new Characteristic::TargetRelativeHumidity(THERMOSTAT_DEFAULT_TARGET_HUMIDITY);
        heatingThreshold = new Characteristic::CoolingThresholdTemperature(TEMPERATURE_INITIAL_VALUE + 2U, true);
        coolingThreshold = new Characteristic::HeatingThresholdTemperature(TEMPERATURE_INITIAL_VALUE, true);
        displayUnits =  new Characteristic::TemperatureDisplayUnits(E_CELSIUS);

        /* Setup the valid values for characteristics */
        currentState->setValidValues(2, E_THERMOSTAT_STATE_OFF, E_THERMOSTAT_STATE_HEAT);
        targetState->setValidValues(3, E_THERMOSTAT_STATE_OFF, E_THERMOSTAT_STATE_HEAT, E_THERMOSTAT_STATE_AUTO);
        displayUnits->setValidValues(1, E_CELSIUS);
        displayUnits->removePerms(PW);

        /* Set the ranges and step values of temperatures */
        targetTemp->setRange(10, 38, 0.5);
        currentTemp->setRange(0, 100, 0.5);
        coolingThreshold->setRange(18, 35, 0.5);
        heatingThreshold->setRange(18, 28, 0.5);

        /* Setup the state of the thermostat */
        lastUpdateTemperature = millis();
        lastSenseTemperature = millis();
        lastUpdateState = millis();
        wasUpdated = false;

        /* In case of a sudden reset, get the last state the heater */
        t_esp01sRelayState currentRelayState = currentState->getHeaterState();
        currentState->setVal(uint8_t(currentRelayState));
        if (currentRelayState == E_ESP01S_RELAY_OPEN) {
            targetState->setVal((int)E_THERMOSTAT_STATE_OFF);
        } else {
            targetState->setVal((int)E_THERMOSTAT_STATE_HEAT);
        }
    }

    /** @brief Update function override */
    boolean update() override {

        /* Check if the user updated any parameter */
        wasUpdated = targetState->updated()      ||
                     targetTemp->updated()       ||
                     coolingThreshold->updated() ||
                     heatingThreshold->updated();

        WEBLOG("User updated the thermostat state");
        return(true);
    }

    /** @brief Loop function override */
    void loop() override {

        /* Update temperature every given duration */
        if ((millis() - lastSenseTemperature) > TEMPERATURE_SENSOR_POLLING_TIME) {
            updateTempReading();
            lastSenseTemperature = millis();
        }

        /* Update current temperature every given duration but only when when you have enough readings */
        if ((millis() - lastUpdateTemperature) > THERMOSTAT_STATUS_UPDATE_POLLING_TIME) {
            updateCurrentTemp();
            lastUpdateTemperature = millis();
        }

        /* Update state every given duration */
        if ((millis() - lastUpdateState) > THERMOSTAT_STATUS_UPDATE_POLLING_TIME || wasUpdated) {
            updateState();
            wasUpdated = false;
            lastUpdateState = millis();
        }
    }

    /* Update the state of the system given parameters */
    void updateState() {
        /* get the current state of the system */
        bool toggle = false;
        bool heaterState = currentState->getVal();

        /* Decide whether to changed the state of the heater based on currently set config */
        switch (targetState->getVal()) {
            case E_THERMOSTAT_STATE_OFF:
                toggle = (bool)E_ESP01S_RELAY_OPEN;
                WEBLOG("State: OFF");
                break;
            case E_THERMOSTAT_STATE_HEAT:
                toggle = toggleManualHeaterState(averageTemp, targetTemp->getVal<float>(), heaterState);
                WEBLOG("State: HEAT mode");
                break;
            case E_THERMOSTAT_STATE_AUTO:
            default:
                toggle = toggleAutoHeaterState(averageTemp, coolingThreshold->getVal<float>(), heatingThreshold->getVal<float>(), heaterState);
                WEBLOG("State: AUTO mode");
        }

        if (toggle != heaterState) {
            WEBLOG("Setting the relay state to %s", toggle == true ? "CLOSE" : "OPEN");
            currentState->setVal(toggle);
        }
    }

    /* Accumulate a new temperature reading using exponential averaging */
    void updateTempReading() {
        /* Read the current temperature */
        float reading = tempHumSensor.getCurrentTemperature();

        /* Validate for correct reading and accumulate if so */
        if ((TEMPERATURE_DEFAULT_MIN_VAL <= reading) &&
            (reading <= TEMPERATURE_DEFAULT_MAX_VAL)) {
            averageTemp *= TEMPERATURE_ALPHA;
            averageTemp += (1 - TEMPERATURE_ALPHA) * reading;
        }
    }

    /* Update the current temperature from accumulated readings */
    void updateCurrentTemp() {
        currentTemp->setVal<float>(averageTemp);
        WEBLOG("Current temperature = %f", averageTemp);
    }

    /* Manual mode Heater Control */
    bool toggleManualHeaterState(float currentTemp, float targetTemp, bool heaterState) {
        bool toggle = heaterState;
        /* Check the temperature range */
        if (currentTemp <= (targetTemp - THERMOSTAT_MANUAL_HYSTERESIS) && heaterState == false) {
            toggle = (bool)E_ESP01S_RELAY_CLOSE;
        } else if (currentTemp >= (targetTemp + THERMOSTAT_MANUAL_HYSTERESIS) && heaterState == true) {
            toggle = (bool)E_ESP01S_RELAY_OPEN;
        } else {
            toggle = heaterState;
        }

        return (toggle);
    }

    /* Auto mode Heater Control */
    bool toggleAutoHeaterState(float currentTemp, float minTemp, float maxTemp, bool heaterState) {
        bool toggle = heaterState;
        /* Check the temperature range */
        if (currentTemp <= (minTemp - THERMOSTAT_AUTO_HYSTERESIS) && heaterState == false) {
            toggle = (bool)E_ESP01S_RELAY_CLOSE;
        } else if (currentTemp >= (maxTemp + THERMOSTAT_AUTO_HYSTERESIS) && heaterState == true) {
            toggle = (bool)E_ESP01S_RELAY_OPEN;
        } else {
            toggle = heaterState;
        }

        return (toggle);
    }
};

#endif /* THERMOSTAT_H */
