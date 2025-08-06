#ifndef MESHTASTICCOMM_H
#define MESHTASTICCOMM_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <string>
#include "DeviceInfo.h"

// Initialisiere die UART-Kommunikation für Meshtastic
void initMeshtasticComm();

// Sende Beacon-Daten an Meshtastic
void sendBeaconToMeshtastic(const std::string& address, DeviceInfo& device, float lastSeenOverride = -1);

// Prüfe auf eingehende Konfigurationsbefehle von Meshtastic
void checkForMeshtasticCommands();

#endif // MESHTASTICCOMM_H