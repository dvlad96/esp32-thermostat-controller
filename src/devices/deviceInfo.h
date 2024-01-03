#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

/************************************************
 *  Includes
 ***********************************************/

/************************************************
 *  Defines / Macros
 ***********************************************/
/* DHT Temperature & Humidity Sensor */
#define DHT_PIN                                 (5U)
#define DHT_TYPE                                (22U)

#define DHT_POLLING_TIME                        (5000U)

#define DHT_TEMPERATURE_DEFAULT_MIN_VAL         (15U)
#define DHT_TEMPERATURE_DEFAULT_MAX_VAL         (30U)

#define DHT_HUMIDITY_DEFAULT_MIN_RANGE          (0U)
#define DHT_HUMIDITY_DEFAULT_MAX_RANGE          (100U)

#define DHT_SENSOR_NAME                         ("Living Room Temperature")
#define DHT_SENSOR_MANUFACTURER                 ("DHT")
#define DHT_SENSOR_MODEL                        ("DHT22")
#define DHT_SENSOR_SERIAL_NUM                   ("SN170332CAE")
#define DHT_SENSOR_FIRMWARE                     ("v1.0.0")

/* Daikin AC Info */
#define DAIKIN_IP_ADDRESS                       ("192.168.1.132")
#define DAIKIN_PORT_ID                          (DEFAULT_DAIKIN_PORT)
#define DAIKIN_AC_NAME                          ("Living Room AC")
#define DAIKIN_AC_MANUFACTURER                  ("Daikin")
#define DAIKIN_AC_MODEL                         ("FTXC35CV1B")
#define DAIKIN_AC_SERIAL_NUM                    ("3P547772-17 C")
#define DAIKIN_AC_FIRMWARE                      ("v1.0.0")

#define DAIKIN_AC_TEMPERATURE_REFRESH           (5000U)

/* Thermostat Info */
#define THERMOSTAT_NAME                         ("Living Room Thermostat")
#define THERMOSTAT_MANUFACTURER                 ("Custom made")
#define THERMOSTAT_MODEL                        ("Model 1")
#define THERMOSTAT_SERIAL_NUM                   ("00000001")
#define THERMOSTAT_FIRMWARE                     ("v1.0.0")

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Public function definition
 ***********************************************/

#endif /* DEVICE_INFO_H */