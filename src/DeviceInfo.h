#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <string>
#include <map>
#include "Filters.h"

// Device information structure
struct DeviceInfo {
  int rssi;
  float rawDistance;
  float filteredDistance;
  float avgRssi;
  float avgDistance;
  std::string name;
  std::string manufacturerId;
  std::string manufacturerName;
  std::string serviceUUID;
  unsigned long lastSeen;
  KalmanFilter kalmanFilter;
  MovingAverageFilter rssiFilter;
  MovingAverageFilter distanceFilter;
  
  DeviceInfo() : 
    rssi(0), 
    rawDistance(0), 
    filteredDistance(0), 
    avgRssi(0), 
    avgDistance(0), 
    lastSeen(0),
    kalmanFilter(0),
    rssiFilter(),
    distanceFilter() {}
};

// Map to store devices and their information
extern std::map<std::string, DeviceInfo> deviceInfoMap;

// Helper functions for device info
String getManufacturerName(uint16_t manufacturerId);
String getServiceName(std::string uuidStr);

#endif // DEVICEINFO_H