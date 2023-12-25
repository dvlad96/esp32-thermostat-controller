#ifndef HEATING_RELAY_H
#define HEATING_RELAY_H

/************************************************
 *  Includes
 ***********************************************/

/************************************************
 *  Defines / Macros
 ***********************************************/
#define HEATING_OFF         (false)
#define HEATING_ON          (true)

/************************************************
 *  Typedef definition
 ***********************************************/

/************************************************
 *  Public function definition
 ***********************************************/

void relayCommunicationSetup();

bool relayOnOff(bool on);

#endif /* HEATING_RELAY_H */