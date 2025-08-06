#include <Arduino.h>
#include "Config.h"
#include "BLEScanner.h"
#include "DeviceInfo.h"
#include "BeaconTracker.h"
#include "JsonUtils.h"
#include "MeshtasticComm.h"
#include "ConfigManager.h"

// Last time JSON was output
unsigned long lastJsonOutput = 0;

void setup() {
  Serial.begin(115200);
  // Warte kurz, aber nicht endlos auf die serielle Verbindung
  delay(1000);
  
  // Initialize ConfigManager first
  ConfigManager::init();
  
  // Try to load saved configuration from NVS (silently fail if not found)
  Serial.println("Checking for saved configuration...");
  ConfigManager::loadFromNVS();
  
  // Initialize UART for Meshtastic communication
  initMeshtasticComm();
  
  Serial.println("\n=====================================================");
  Serial.println("BLE Beacon Scanner mit dynamischer Konfiguration");
  Serial.println("=====================================================");
  Serial.printf("Gateway ID: %s\n", GATEWAY_ID.c_str());
  Serial.println("JSON Command Format: {\"target\":\"" + String(GATEWAY_ID) + "\",\"parameter\":value}");
  Serial.printf("Distanz-Schwellenwert: %.1f Meter\n", ConfigManager::getDistanceThreshold());
  Serial.printf("TX Power: %d dBm @ 1m\n", ConfigManager::getTxPower());
  Serial.printf("Environmental Factor: %.1f (%.1f=Freiraum, >2.0=mit Hindernissen)\n", 
                ConfigManager::getEnvironmentalFactor(), 2.0);
  Serial.printf("Distanzkorrektur: %.1f meter\n", ConfigManager::getDistanceCorrection());
  Serial.printf("Gleitender Mittelwert: %d Werte\n", ConfigManager::getWindowSize());
  Serial.printf("JSON-Ausgabe-Intervall: %d ms\n", JSON_OUTPUT_INTERVAL);
  Serial.printf("Beacon Timeout: %d Sekunden\n", ConfigManager::getBeaconTimeout());
  Serial.printf("UART für Meshtastic: TX=%d, RX=%d, Baudrate=%d\n", UART_TX_PIN, UART_RX_PIN, UART_BAUD_RATE);
  Serial.println("Beacon-Verschwinden-Erkennung: Aktiv");
  Serial.println("Gateway Targeting System: Aktiv");
  Serial.println("Dynamische Konfiguration über Meshtastic: Aktiv");
  Serial.println("Bereit zum Empfang von Konfigurationsbefehlen über Meshtastic UART...");
  
  // Initialize device filter (now managed by ConfigManager)
  parseDeviceFilter();
  
  Serial.println("=====================================================");
  
  // Initialize BLE scanner
  bleScanner.init();
  
  Serial.println("BLE Scanner initialisiert.");
  
  Serial.println("Starte Scannen nach BLE-Geräten in der Umgebung...");
  
  // Initialize tracking variables
  initBeaconTracking();
  
  // Initialize timing
  lastJsonOutput = millis();
}

void loop() {
  // Check for incoming Meshtastic configuration commands
  checkForMeshtasticCommands();
  
  // Reset counter for devices in range
  devicesInRangeCount = 0;
  
  // Start scanning
  int deviceCount = bleScanner.scan();
  
  // Count devices within threshold (using dynamic threshold)
  for (auto const& pair : deviceInfoMap) {
    std::string address = pair.first;
    DeviceInfo device = pair.second;
    
    // Skip devices not in our filter (if filter is active)
    if (!isDeviceInFilter(address)) {
      continue;
    }
    
    if (device.filteredDistance <= ConfigManager::getDistanceThreshold() && 
        (millis() - device.lastSeen) < 30000) {
      devicesInRangeCount++;
    }
  }
  
  // Find and track closest beacon for UART output
  findAndTrackClosestBeacon();
  
  // Print brief summary to serial
  Serial.print("Geräte gefunden: ");
  Serial.print(deviceCount);
  Serial.print(" (");
  Serial.print(devicesInRangeCount);
  Serial.println(" innerhalb Schwellenwert)");
  
  // Print status at regular intervals
  static unsigned long lastStatusPrint = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - lastStatusPrint > 10000) {
    lastStatusPrint = currentMillis;
    
    if (!getCurrentClosestBeaconAddress().empty()) {
      Serial.print("Aktuell verfolgter Beacon: ");
      Serial.print(getCurrentClosestBeaconAddress().c_str());
      Serial.print(" (Distanz: ");
      Serial.print(getCurrentClosestBeaconDistance());
      Serial.println(" m)");
    } else {
      Serial.println("Kein Beacon aktuell verfolgt");
    }
    
    // Show current threshold and gateway info for reference
    Serial.printf("Gateway: %s, Distanz-Schwellenwert: %.2fm\n", GATEWAY_ID.c_str(), ConfigManager::getDistanceThreshold());
  }
  
  // Output detailed JSON at intervals to serial
  if (millis() - lastJsonOutput >= JSON_OUTPUT_INTERVAL) {
    outputDevicesAsJson();
    lastJsonOutput = millis();
  }
  
  // Clear scan results to free memory
  bleScanner.clearResults();
}