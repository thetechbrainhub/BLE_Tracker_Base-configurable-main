#include "BeaconTracker.h"
#include "Config.h"
#include "DeviceInfo.h"
#include "MeshtasticComm.h"
#include <Arduino.h>

// Beacon Tracking Variablen
static std::string currentClosestBeaconAddress = "";
static float currentClosestBeaconDistance = 999.0;
static bool beaconStatusChanged = false;
static unsigned long lastBeaconUpdate = 0;
static bool beaconDisappearanceReported = false;  // Flag um zu tracken, ob das Verschwinden bereits gemeldet wurde
static std::set<std::string> currentScanBeacons;  // Set für die im aktuellen Scan gefundenen Beacons

// Getter und Setter Implementierungen
const std::string& getCurrentClosestBeaconAddress() {
  return currentClosestBeaconAddress;
}

float getCurrentClosestBeaconDistance() {
  return currentClosestBeaconDistance;
}

bool getBeaconStatusChanged() {
  return beaconStatusChanged;
}

bool getBeaconDisappearanceReported() {
  return beaconDisappearanceReported;
}

std::set<std::string>& getCurrentScanBeacons() {
  return currentScanBeacons;
}

void setCurrentClosestBeaconAddress(const std::string& address) {
  currentClosestBeaconAddress = address;
}

void setCurrentClosestBeaconDistance(float distance) {
  currentClosestBeaconDistance = distance;
}

void setBeaconStatusChanged(bool changed) {
  beaconStatusChanged = changed;
}

void setBeaconDisappearanceReported(bool reported) {
  beaconDisappearanceReported = reported;
}

void clearCurrentScanBeacons() {
  currentScanBeacons.clear();
}

void updateLastBeaconSeen() {
  lastBeaconUpdate = millis();
}

unsigned long getLastBeaconUpdate() {
  return lastBeaconUpdate;
}

void initBeaconTracking() {
  currentClosestBeaconAddress = "";
  currentClosestBeaconDistance = 999.0;
  beaconStatusChanged = false;
  beaconDisappearanceReported = false;
  currentScanBeacons.clear();
  lastBeaconUpdate = 0;
}

// Find the closest beacon and handle tracking
void findAndTrackClosestBeacon() {
  std::string closestBeaconAddress = "";
  float closestBeaconDistance = 999.0;
  DeviceInfo* closestBeacon = nullptr;
  
  // Debug-Ausgabe zum Beginn der Funktion
  Serial.println("UART-DEBUG: Suche nach dem nächsten Beacon...");
  Serial.println("UART-DEBUG: Aktuell verfolgter Beacon: " + String(currentClosestBeaconAddress.empty() ? "keiner" : currentClosestBeaconAddress.c_str()));
  
  // Lösche die Liste der aktuell sichtbaren Beacons
  currentScanBeacons.clear();
  
  // Find the closest beacon and handle tracking
  for (auto& pair : deviceInfoMap) {
    std::string address = pair.first;
    DeviceInfo& device = pair.second;
    
    // Skip devices not in our filter (if filter is active)
    if (!isDeviceInFilter(address)) {
      continue;
    }
    
    // Only consider devices within threshold and seen in the last 30 seconds
    if (device.filteredDistance <= DISTANCE_THRESHOLD && 
        (millis() - device.lastSeen) < 30000) {
      
      // Add to the set of currently visible beacons
      currentScanBeacons.insert(address);
      
      // Check if this is the closest beacon
      if (device.filteredDistance < closestBeaconDistance) {
        closestBeaconDistance = device.filteredDistance;
        closestBeaconAddress = address;
        closestBeacon = &device;
      }
    }
  }
  
  // Überprüfen, ob der aktuell verfolgte Beacon verschwunden ist
  if (!currentClosestBeaconAddress.empty()) {
    bool beaconIsVisible = currentScanBeacons.find(currentClosestBeaconAddress) != currentScanBeacons.end();
    
    Serial.print("UART-DEBUG: Verfolgter Beacon ");
    Serial.print(currentClosestBeaconAddress.c_str());
    Serial.print(beaconIsVisible ? " ist sichtbar" : " ist NICHT sichtbar");
    Serial.print(", Verschwinden gemeldet: ");
    Serial.println(beaconDisappearanceReported ? "Ja" : "Nein");
    
    // Wenn der Beacon gerade verschwunden ist und wir es noch nicht gemeldet haben
    if (!beaconIsVisible && !beaconDisappearanceReported) {
      Serial.println("UART-DEBUG: *** BEACON IST VERSCHWUNDEN! ***");
      Serial.println("UART-DEBUG: Sende finale Benachrichtigung mit presence: false");
      
      if (deviceInfoMap.find(currentClosestBeaconAddress) != deviceInfoMap.end()) {
        DeviceInfo& currentBeacon = deviceInfoMap[currentClosestBeaconAddress];
        
        // Sende eine spezielle Nachricht mit presence=false
        sendBeaconToMeshtastic(currentClosestBeaconAddress, currentBeacon, BEACON_TIMEOUT_SECONDS + 1);
        
        // Markiere, dass wir das Verschwinden bereits gemeldet haben
        beaconDisappearanceReported = true;
        
        Serial.println("UART-DEBUG: Verschwinden-Nachricht wurde gesendet");
      } else {
        Serial.println("UART-DEBUG: FEHLER: Beacon-Info nicht gefunden!");
      }
    }
    
    // Wenn der Beacon wieder auftaucht, nachdem wir sein Verschwinden gemeldet haben
    if (beaconIsVisible && beaconDisappearanceReported) {
      Serial.println("UART-DEBUG: BEACON IST ZURÜCKGEKEHRT! Sende neue Nachricht mit presence: true");
      
      // Dies ist ein Status-Wechsel und muss gemeldet werden
      beaconStatusChanged = true;
      beaconDisappearanceReported = false;
      
      // Wenn es der aktuell verfolgte Beacon ist, sofort ein Update senden
      if (deviceInfoMap.find(currentClosestBeaconAddress) != deviceInfoMap.end()) {
        DeviceInfo& currentBeacon = deviceInfoMap[currentClosestBeaconAddress];
        sendBeaconToMeshtastic(currentClosestBeaconAddress, currentBeacon);
        beaconStatusChanged = false; // Reset nach dem Senden
        Serial.println("UART-DEBUG: Rückkehr-Nachricht wurde gesendet");
      }
    }
  }
  
  // Check if we found a closest beacon
  if (closestBeacon != nullptr) {
    // Wenn wir einen anderen Beacon verfolgen als den aktuell nächsten
    if (!currentClosestBeaconAddress.empty() && currentClosestBeaconAddress != closestBeaconAddress) {
      Serial.print("UART-DEBUG: Neuer nächster Beacon gefunden. Alt: ");
      Serial.print(currentClosestBeaconAddress.c_str());
      Serial.print(" -> Neu: ");
      Serial.println(closestBeaconAddress.c_str());
      
      // Now update to the new closest beacon
      beaconStatusChanged = true;
      currentClosestBeaconAddress = closestBeaconAddress;
      currentClosestBeaconDistance = closestBeaconDistance;
      lastBeaconUpdate = millis();
      
      // Reset the disappearance flag when tracking a new beacon
      beaconDisappearanceReported = false;
      Serial.println("UART-DEBUG: Verschwinden-Flag zurückgesetzt für neuen Beacon");
      
    } else if (currentClosestBeaconAddress.empty()) {
      // First time detecting a beacon
      Serial.println("UART-DEBUG: Erster Beacon entdeckt!");
      beaconStatusChanged = true;
      currentClosestBeaconAddress = closestBeaconAddress;
      currentClosestBeaconDistance = closestBeaconDistance;
      lastBeaconUpdate = millis();
      
      // Reset the disappearance flag
      beaconDisappearanceReported = false;
    } 
    // Wenn wir bereits den gleichen Beacon verfolgen (bereits existierender Fall)
    else if (currentClosestBeaconAddress == closestBeaconAddress) {
      // Update die aktuelle Distanz
      currentClosestBeaconDistance = closestBeaconDistance;
      lastBeaconUpdate = millis();
    }
    
    // If a status change occurred or we need to send regular updates
    if (beaconStatusChanged) {
      // Send data for the new closest beacon
      Serial.println("UART-DEBUG: Sende Daten für nächsten Beacon...");
      sendBeaconToMeshtastic(closestBeaconAddress, *closestBeacon);
      beaconStatusChanged = false;
    }
  } else {
    // No beacons found within threshold
    Serial.println("UART-DEBUG: Kein Beacon innerhalb des Schwellenwerts gefunden.");
    
    // Wenn kein Beacon mehr sichtbar ist und wir einen verfolgt haben, aber noch keine Verschwinden-Meldung gesendet haben
    if (!currentClosestBeaconAddress.empty() && currentScanBeacons.empty() && !beaconDisappearanceReported) {
      Serial.println("UART-DEBUG: Alle Beacons sind verschwunden!");
      
      // Nur wenn wir die deviceInfo noch haben
      if (deviceInfoMap.find(currentClosestBeaconAddress) != deviceInfoMap.end()) {
        DeviceInfo& lastTrackedBeacon = deviceInfoMap[currentClosestBeaconAddress];
        
        // Verschwinden melden
        Serial.println("UART-DEBUG: Sende finale Benachrichtigung für letzten Beacon mit presence: false");
        sendBeaconToMeshtastic(currentClosestBeaconAddress, lastTrackedBeacon, BEACON_TIMEOUT_SECONDS + 1);
        
        beaconDisappearanceReported = true;
        Serial.println("UART-DEBUG: Verschwinden-Nachricht wurde gesendet");
      }
    }
  }
  
  // Debug-Ausgabe zum Ende der Funktion
  Serial.print("UART-DEBUG: Aktuell sichtbare Beacons: ");
  Serial.println(currentScanBeacons.size());
  Serial.print("UART-DEBUG: Verfolgter Beacon: ");
  Serial.println(currentClosestBeaconAddress.empty() ? "keiner" : currentClosestBeaconAddress.c_str());
  Serial.print("UART-DEBUG: Status-Change-Flag: ");
  Serial.println(beaconStatusChanged ? "Ja" : "Nein");
  Serial.print("UART-DEBUG: Verschwinden bereits gemeldet: ");
  Serial.println(beaconDisappearanceReported ? "Ja" : "Nein");
  Serial.println("UART-DEBUG: Ende der Beacon-Tracking-Funktion");
}