#include "MeshtasticComm.h"
#include "Config.h"
#include "JsonUtils.h"
#include "ConfigManager.h"

// UART für Meshtastic
HardwareSerial MeshtasticSerial(1); // Use UART1

void initMeshtasticComm() {
  // Initialisiere UART für Meshtastic-Kommunikation
  MeshtasticSerial.begin(UART_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  Serial.println("Meshtastic UART initialized - ready to receive config commands");
}

void sendBeaconToMeshtastic(const std::string& address, DeviceInfo& device, float lastSeenOverride) {
  String json = generateBeaconJSON(address, device, lastSeenOverride);
  
  // Debug-Ausgabe vor dem Senden
  Serial.println("----------------------------------------");
  Serial.println("UART-DEBUG: Sende Beacon-Daten an Meshtastic:");
  Serial.println(json);
  Serial.println("----------------------------------------");
  
  // Sende über UART
  MeshtasticSerial.println(json);
  
  // Debug-Bestätigung
  Serial.println("UART-DEBUG: Daten gesendet!");
}

void checkForMeshtasticCommands() {
  if (MeshtasticSerial.available()) {
    String receivedData = MeshtasticSerial.readStringUntil('\n');
    receivedData.trim(); // Remove whitespace and newlines
    
    if (receivedData.length() > 0) {
      Serial.println("========================================");
      Serial.println("UART-DEBUG: Received from Meshtastic:");
      Serial.println(receivedData);
      Serial.println("========================================");
      
      // Extract JSON from the received data
      String jsonData = "";
      int jsonStart = receivedData.indexOf('{');
      int jsonEnd = receivedData.lastIndexOf('}');
      
      if (jsonStart >= 0 && jsonEnd > jsonStart) {
        jsonData = receivedData.substring(jsonStart, jsonEnd + 1);
        Serial.println("UART-DEBUG: Extracted JSON: " + jsonData);
        
        Serial.println("UART-DEBUG: Processing as JSON config command...");
        
        bool success = ConfigManager::processConfigCommand(jsonData);
        
        if (success) {
          Serial.println("UART-DEBUG: Configuration updated successfully!");
          
          // Send acknowledgment back to Meshtastic
          String ack = "{\"status\":\"ok\",\"message\":\"config updated\"}";
          MeshtasticSerial.println(ack);
          Serial.println("UART-DEBUG: Acknowledgment sent: " + ack);
          
        } else {
          Serial.println("UART-DEBUG: Configuration update failed!");
          
          // Send error acknowledgment
          String error = "{\"status\":\"error\",\"message\":\"invalid config\"}";
          MeshtasticSerial.println(error);
          Serial.println("UART-DEBUG: Error response sent: " + error);
        }
      } else {
        Serial.println("UART-DEBUG: No valid JSON found in received data - ignoring");
        Serial.println("UART-DEBUG: Looking for '{' at position " + String(jsonStart) + " and '}' at position " + String(jsonEnd));
      }
    }
  }
}