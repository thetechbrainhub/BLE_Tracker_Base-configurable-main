#ifndef JSONUTILS_H
#define JSONUTILS_H

#include <Arduino.h>
#include <string>
#include "DeviceInfo.h"

// Generate JSON for a specific beacon
String generateBeaconJSON(const std::string& address, DeviceInfo& device, float lastSeenOverride = -1);

// Generate JSON for all devices within threshold
String generateDevicesJSON();

// Output formatted JSON data to Serial
void outputDevicesAsJson();

#endif // JSONUTILS_H