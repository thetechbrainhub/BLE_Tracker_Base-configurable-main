#include "DeviceInfo.h"
#include <Arduino.h>

// Global device map initialization
std::map<std::string, DeviceInfo> deviceInfoMap;

// Get manufacturer name from ID
String getManufacturerName(uint16_t manufacturerId) {
  switch (manufacturerId) {
    case 0x004C: return "Apple";
    case 0x0059: return "Nordic";
    case 0x0006: return "Microsoft";
    case 0x00E0: return "Google";
    case 0x012D: return "Sony";
    case 0x038F: return "Xiaomi";
    case 0x01AE: return "Samsung";
    default: return "Unknown";
  }
}

// Get service name from UUID
String getServiceName(std::string uuidStr) {
  if (uuidStr.find("1800") != std::string::npos) {
    return "Generic Access Profile";
  } else if (uuidStr.find("1801") != std::string::npos) {
    return "Generic Attribute Profile";
  } else if (uuidStr.find("180F") != std::string::npos) {
    return "Battery Service";
  } else if (uuidStr.find("180A") != std::string::npos) {
    return "Device Information Service";
  } else if (uuidStr.find("FEAA") != std::string::npos) {
    return "Eddystone Beacon";
  } else if (uuidStr.find("FD6F") != std::string::npos) {
    return "Exposure Notification Service";
  } else {
    return "Unknown Service";
  }
}