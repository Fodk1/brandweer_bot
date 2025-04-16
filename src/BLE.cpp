#include <ArduinoBLE.h>

#include "turretMovement.h"
#include "flags.h"

// Define a custom service matching the iOS app expectation
BLEService controlService("FFE0");  // Must match iOS serviceUUID

// Custom characteristic for sending/receiving data
// Change BLENotify | BLERead | BLEWrite to include BLEWriteWithoutResponse if needed
// But for this fix, we'll use BLEWrite since iOS is using .withResponse
BLECharacteristic controlCharacteristic("FFE1", BLENotify | BLERead | BLEWrite, 40);

// Variables to track connection
bool connected = false;
unsigned long lastActivityTime = 0;
const unsigned long CONNECTION_TIMEOUT = 60000; // 1 minute timeout

void onBLEConnected(BLEDevice central);
void onBLEDisconnected(BLEDevice central);
void onCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic);

void BLEInit() {

	// Initialize BLE with retry mechanism
	int retryCount = 0;
	while (!BLE.begin() && retryCount < 5) {
		delay(1000);
		retryCount++;
	}

	if (retryCount >= 5) {
		delay(1000);
		NVIC_SystemReset();
		return;
	}

	// Set the device name - must contain "Portenta" for iOS app to find it
	BLE.setDeviceName("Portenta Controller");
	BLE.setLocalName("Portenta Controller");

	// Set up the custom service
	BLE.setAdvertisedService(controlService);

	// Add the characteristic to the service
	controlService.addCharacteristic(controlCharacteristic);

	// Add the service
	BLE.addService(controlService);

	// Set up a characteristic value changed handler
	controlCharacteristic.setEventHandler(BLEWritten, onCharacteristicWritten);

	// Set event handlers for connection events
	BLE.setEventHandler(BLEConnected, onBLEConnected);
	BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);

	BLE.advertise();

}

void BLEUpdate() {
	// Poll for BLE events
	BLE.poll();

	// Connection status monitoring
	if (connected) {
		unsigned long currentTime = millis();
		if (currentTime - lastActivityTime > 5000) {
		lastActivityTime = currentTime;
		}
	}
}

void onBLEConnected(BLEDevice central) {
	Serial.println(central.address());
	connected = true;
	lastActivityTime = millis();
}

void onBLEDisconnected(BLEDevice central) {
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
	if (data.startsWith("X:")) {
		//X:x
		int x = data.substring(data.indexOf(":")).toFloat();
		turretSetXMovement(x);
	}
	else if(data.startsWith("Y:")){
		int y = data.substring(data.indexOf(":")).toFloat();
		turretSetYMovement(y);
	}
	else if (data.startsWith("A")) {
		// manual mode?
		bool isOn = flags.get() & MANUAL_CONTROL_FLAG;
		flags.clear();

		if (!isOn)
			flags.set(MANUAL_CONTROL_FLAG);
	}
}