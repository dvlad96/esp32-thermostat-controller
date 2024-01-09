#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

/************************************************
 *  Includes
 ***********************************************/

/************************************************
 *  Defines / Macros
 ***********************************************/
/* Temperature & Humidity Sensor */
#define TEMPERATURE_SENSOR_POLLING_TIME         (5000U)
#define TEMPERATURE_DEFAULT_MIN_VAL             (15U)
#define TEMPERATURE_DEFAULT_MAX_VAL             (30U)
#define HUMIDITY_DEFAULT_MIN_RANGE              (0U)
#define HUMIDITY_DEFAULT_MAX_RANGE              (100U)

#define TEMP_HUM_SENSOR_NAME                    ("Living Room Temperature")
#define TEMP_HUM_SENSOR_MANUFACTURER            ("Adafruit")
#define TEMP_HUM_SENSOR_MODEL                   ("AHT20")
#define TEMP_HUM_SENSOR_SERIAL_NUM              ("SN170332CAE")
#define TEMP_HUM_SENSOR_FIRMWARE                ("v1.0.0")

/* Thermostat Info */
#define THERMOSTAT_NAME                         ("Living Room Thermostat")
#define THERMOSTAT_MANUFACTURER                 ("Custom made")
#define THERMOSTAT_MODEL                        ("Model 1")
#define THERMOSTAT_SERIAL_NUM                   ("00000001")
#define THERMOSTAT_FIRMWARE                     ("v1.0.0")

#define THERMOSTAT_RELAY_IP_ADDRESS             ("192.168.1.100")
#define THERMOSTAT_RELAY_PORT_ID                (80U)

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Public function definition
 ***********************************************/

#endif /* DEVICE_INFO_H */