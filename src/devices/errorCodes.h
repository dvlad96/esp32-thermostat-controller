#ifndef ERROR_CODES_H
#define ERROR_CODES_H

/************************************************
 *  Includes
 ***********************************************/

/************************************************
 *  Defines / Macros
 ***********************************************/

/************************************************
 *  Typedef definition
 ***********************************************/
/** @brief This enum represents all the possible HTTP Requests failures*/
typedef enum {
    E_REQUEST_SUCCESS = 0U,             /**< HTTP Request success */
    E_REQUEST_FAILURE = 1U              /**< HTTP Request failure */
} t_httpErrorCodes;

/** @brief This enum represents all the possible ESP-NOW Error codes */
typedef enum {
    E_COMMAND_SUCCESS = 0U,             /**< ESP-Now Command success */
    E_COMMAND_FAILURE = 1U              /**< ESP-Now Command failure */
} t_espNowErrorCodes;

/************************************************
 *  Public function definition
 ***********************************************/

#endif /* ERROR_CODES_H */
