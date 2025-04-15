#include <ArduinoBLE.h>

// Define a custom service matching the iOS app expectation
BLEService controlService("FFE0");  // Must match iOS serviceUUID

// Custom characteristic for sending/receiving data
// Change BLENotify | BLERead | BLEWrite to include BLEWriteWithoutResponse if needed
// But for this fix, we'll use BLEWrite since iOS is using .withResponse
BLECharacteristic controlCharacteristic("FFE1", BLENotify | BLERead | BLEWrite, 20);

// Variables to track connection
bool connected = false;
unsigned long lastActivityTime = 0;
const unsigned long CONNECTION_TIMEOUT = 60000; // 1 minute timeout

// void setup() {
//   // Initialize serial communication at higher baud rate
//   Serial.begin(115200);
//   delay(1500);
  
//   Serial.println("Portenta H7 Controller - Custom BLE Mode");
  
//   // Initialize BLE with retry mechanism
//   int retryCount = 0;
//   while (!BLE.begin() && retryCount < 5) {
//     Serial.println("Failed to initialize BLE! Retrying...");
//     delay(1000);
//     retryCount++;
//   }
  
//   if (retryCount >= 5) {
//     Serial.println("Could not initialize BLE after multiple attempts. Restarting...");
//     delay(1000);
//     NVIC_SystemReset();
//     return;
//   }
  
//   // Set the device name - must contain "Portenta" for iOS app to find it
//   BLE.setDeviceName("Portenta Controller");
//   BLE.setLocalName("Portenta Controller");
  
//   // Set up the custom service
//   BLE.setAdvertisedService(controlService);
  
//   // Add the characteristic to the service
//   controlService.addCharacteristic(controlCharacteristic);
  
//   // Add the service
//   BLE.addService(controlService);
  
//   // Set up a characteristic value changed handler
//   controlCharacteristic.setEventHandler(BLEWritten, onCharacteristicWritten);
  
//   // Set event handlers for connection events
//   BLE.setEventHandler(BLEConnected, onBLEConnected);
//   BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);
  
//   BLE.advertise();
  
//   Serial.println("Advertising as Portenta Controller");
//   Serial.println("The device should now appear in your iPhone's Bluetooth scan");
//   Serial.println("Characteristic properties: BLENotify | BLERead | BLEWrite");
// }

// void loop() {
//   // Poll for BLE events
//   BLE.poll();
  
//   // Connection status monitoring
//   if (connected) {
//     unsigned long currentTime = millis();
//     if (currentTime - lastActivityTime > 5000) {
//       Serial.println("Connection active...");
//       lastActivityTime = currentTime;
//     }
//   }
// }

void onBLEConnected(BLEDevice central) {
  Serial.print("Connected to: ");
  Serial.println(central.address());
  connected = true;
  lastActivityTime = millis();
  
  // Added to debug
  Serial.println("Ready to receive commands!");
}

void onBLEDisconnected(BLEDevice central) {
  Serial.print("Disconnected from: ");
  Serial.println(central.address());
  Serial.println("Readvertising...");
  connected = false;
  
  // Start advertising again after disconnection
  BLE.advertise();
}

// Enhanced function to handle data received from iOS app
void onCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  // Get the raw data first to debug what's actually being received
  const uint8_t* bytes = characteristic.value();
  int length = characteristic.valueLength();
  
  // Convert to string
  String data = "";
  for (int i = 0; i < length; i++) {
    data += (char)bytes[i];
  }
  
  // Parse the command
  if (data.startsWith("J:")) {
    // Joystick command format: "J:L:x:y" or "J:R:x:y"
    int firstColon = data.indexOf(':');
    int secondColon = data.indexOf(':', firstColon + 1);
    int thirdColon = data.indexOf(':', secondColon + 1);
    
    if (firstColon >= 0 && secondColon >= 0 && thirdColon >= 0) {
      String joystick = data.substring(firstColon + 1, secondColon);
      int x = data.substring(secondColon + 1, thirdColon).toInt();
      int y = data.substring(thirdColon + 1).toInt();
      
      bool isLeft = (joystick == "L");
      
      // Process joystick input
      handleJoystickInput(isLeft, x, y);
      
      // Send acknowledgment
      String response = "ACK:" + data;
      controlCharacteristic.writeValue(response.c_str(), response.length());
    } else {
      Serial.println("Error: Invalid joystick data format");
    }
  } 
  else if (data.startsWith("B:")) {
    // Button command format: "B:a" or "B:b", etc.
    if (data.length() > 2) {
      String button = data.substring(2);
      
      Serial.print("Parsed button: ");
      Serial.println(button);
      
      // Process button input
      handleButtonInput(button);
      
      // Send acknowledgment
      String response = "ACK:" + data;
      controlCharacteristic.writeValue(response.c_str(), response.length());
    } else {
      Serial.println("Error: Invalid button data format");
    }
  } else {
    Serial.println("Error: Unknown command format");
  }
}

// Handle joystick input for your car application
void handleJoystickInput(bool isLeft, int x, int y) {
  if (isLeft) {
    // Left joystick - perhaps controls steering?
    Serial.print(x);
    Serial.println(y);
    
    // Your car control code here
    // Example: controlSteering(x);
  } else {
    // Right joystick - perhaps controls acceleration/braking?
    Serial.print(x);
    Serial.println(y);
    
    // Your car control code here
    // Example: controlThrottle(y);
  }
}

// Handle button input for your car application
void handleButtonInput(String button) {
  
  // Your button handling code here
}