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
  Serial.printf("Gateway ID: %s - Only processing commands with matching target field\n", GATEWAY_ID.c_str());
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
        
        // Parse JSON to check target field properly
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, jsonData);
        
        if (!error && doc.containsKey("target")) {
          String targetGateway = doc["target"].as<String>();
          
          if (targetGateway == GATEWAY_ID) {
            Serial.println("UART-DEBUG: Message for this gateway (" + String(GATEWAY_ID) + ") - processing...");
            
            bool success = ConfigManager::processConfigCommand(jsonData);
            
            if (success) {
              Serial.println("UART-DEBUG: Configuration updated successfully!");
              
              // Send short acknowledgment back to Meshtastic
              String ack = "{\"ack\":\"" + String(GATEWAY_ID) + "\",\"ok\":true}";
              MeshtasticSerial.println(ack);
              Serial.println("UART-DEBUG: Acknowledgment sent: " + ack);
              
            } else {
              Serial.println("UART-DEBUG: Configuration update failed!");
              
              // Send short error acknowledgment
              String error = "{\"ack\":\"" + String(GATEWAY_ID) + "\",\"ok\":false}";
              MeshtasticSerial.println(error);
              Serial.println("UART-DEBUG: Error response sent: " + error);
            }
          } else {
            Serial.println("UART-DEBUG: Message not for this gateway (" + String(GATEWAY_ID) + ") - target: " + targetGateway);
          }
        } else {
          Serial.println("UART-DEBUG: JSON parsing failed or no target field found - ignoring");
        }
      } else {
        Serial.println("UART-DEBUG: No valid JSON found in received data - ignoring");
        Serial.println("UART-DEBUG: Looking for '{' at position " + String(jsonStart) + " and '}' at position " + String(jsonEnd));
      }
    }
  }
}