#ifndef BLESCANNER_H
#define BLESCANNER_H

#include <NimBLEDevice.h>
#include <NimBLEUtils.h>
#include <NimBLEScan.h>
#include <NimBLEAdvertisedDevice.h>
#include "DeviceInfo.h"

// Callback for BLE scan results
class MyAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
  void onResult(NimBLEAdvertisedDevice* advertisedDevice);
};

// BLEScanner class to handle scanning operations
class BLEScanner {
private:
  NimBLEScan* pBLEScan;

public:
  BLEScanner();
  void init();
  int scan();  // Returns the number of devices found
  void clearResults();
};

// Global scanner instance
extern BLEScanner bleScanner;

#endif // BLESCANNER_H