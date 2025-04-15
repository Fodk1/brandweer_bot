// #include <ArduinoBLE.h>

// #define MOTOR1_DIR_PIN D20
// #define MOTOR1_SPEED_PIN D4 //Fl
// #define MOTOR2_DIR_PIN A3
// #define MOTOR2_SPEED_PIN D5 //Fr
// #define MOTOR3_DIR_PIN D21
// #define MOTOR3_SPEED_PIN D2 //Bl
// #define MOTOR4_DIR_PIN A4
// #define MOTOR4_SPEED_PIN D3 //Br

// int motorsEn[4] = {
//     MOTOR1_SPEED_PIN,
//     MOTOR2_SPEED_PIN,
//     MOTOR3_SPEED_PIN,
//     MOTOR4_SPEED_PIN
// };

// int motorsDir[4] = {
//     MOTOR1_DIR_PIN,
//     MOTOR2_DIR_PIN,
//     MOTOR3_DIR_PIN,
//     MOTOR4_DIR_PIN
// };

// // Define a custom service matching the iOS app expectation
// BLEService controlService("FFE0");  // Must match iOS serviceUUID

// // Custom characteristic for sending/receiving data
// // Change BLENotify | BLERead | BLEWrite to include BLEWriteWithoutResponse if needed
// // But for this fix, we'll use BLEWrite since iOS is using .withResponse
// BLECharacteristic controlCharacteristic("FFE1", BLENotify | BLERead | BLEWrite, 40);

// // Variables to track connection
// bool connected = false;
// unsigned long lastActivityTime = 0;
// const unsigned long CONNECTION_TIMEOUT = 60000; // 1 minute timeout

// void onBLEConnected(BLEDevice central);
// void onBLEDisconnected(BLEDevice central);
// void onCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic);

// void BLEInit() {

//     for (size_t i = 0; i < 4; i++){
//         pinMode(motorsEn[i], OUTPUT);    
//         pinMode(motorsDir[i], OUTPUT);    
//     }

//   // Initialize BLE with retry mechanism
//   int retryCount = 0;
//   while (!BLE.begin() && retryCount < 5) {
//     delay(1000);
//     retryCount++;
//   }
  
//   if (retryCount >= 5) {
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
  
// }

// void BLEUpdate() {
//   // Poll for BLE events
//   BLE.poll();
  
//   // Connection status monitoring
//   if (connected) {
//     unsigned long currentTime = millis();
//     if (currentTime - lastActivityTime > 5000) {
//       lastActivityTime = currentTime;
//     }
//   }
// }

// void onBLEConnected(BLEDevice central) {
//   Serial.println(central.address());
//   connected = true;
//   lastActivityTime = millis();
// }

// void onBLEDisconnected(BLEDevice central) {
//   connected = false;
  
//   // Start advertising again after disconnection
//   BLE.advertise();
// }

// // Enhanced function to handle data received from iOS app
// void onCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
//   // Get the raw data first to debug what's actually being received
//   const uint8_t* bytes = characteristic.value();
//   int length = characteristic.valueLength();
  
//   // Convert to string
//   String data = "";
//   for (int i = 0; i < length; i++) {
//     data += (char)bytes[i];
//   }
  
//   // Parse the command
//   if (data.startsWith("E:")) {
//     //E:lf:lf:rf:lb:rb

//     int prevColon = data.indexOf(':');

//     for (size_t i = 0; i < 1; i++)
//     {
//         int nextColon = data.indexOf(':', prevColon + 1);

//         int speed = data.substring(prevColon + 1, nextColon).toInt();
//         for (size_t n = 0; n < 4; n++)
//         {
//             analogWrite(motorsEn[n], speed);
//         }
//         prevColon = nextColon;
//     }

//     for (size_t i = 0; i < 4; i++)
//     {
//         int nextColon = data.indexOf(':', prevColon + 1);

//         int dir = 0;
//         if (nextColon > 0)
//             dir = data.substring(prevColon + 1, nextColon).toInt();
//         else
//             dir = data.substring(prevColon + 1).toInt();

//         digitalWrite(motorsDir[i], dir);
//         prevColon = nextColon;

//     }
//   } 
//   else if (data.startsWith("B:")) {
//     // Button command format: "B:a" or "B:b", etc.
//     if (data.length() > 2) {
//       String button = data.substring(2);
      
//       Serial.print("Parsed button: ");
//       Serial.println(button);
      
//       // Send acknowledgment
//       String response = "ACK:" + data;
//       controlCharacteristic.writeValue(response.c_str(), response.length());
//     }
//   }
// }