#include <Bluepad32.h>
#include "motionPlatform.h"

#define MOTOR1_DIR_PIN 16
#define MOTOR1_SPEED_PIN D2
#define MOTOR2_DIR_PIN 17
#define MOTOR2_SPEED_PIN D3
#define MOTOR3_DIR_PIN D20
#define MOTOR3_SPEED_PIN D4
#define MOTOR4_DIR_PIN 19
#define MOTOR4_SPEED_PIN D5

// Movement test durations
const int MOVE_DURATION = 2000;  // Duration for each movement test (ms)
const int PAUSE_DURATION = 1000; // Pause between tests (ms)

// Test mode flag - set to true to run tests, false for normal controller operation
bool testMode = false;

MotionControl motionControl(
    MOTOR1_DIR_PIN, MOTOR1_SPEED_PIN,
    MOTOR2_DIR_PIN, MOTOR2_SPEED_PIN,
    MOTOR3_DIR_PIN, MOTOR3_SPEED_PIN,
    MOTOR4_DIR_PIN, MOTOR4_SPEED_PIN
);

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// Function prototypes for test functions
void testBasicMovements();
void testDiagonalMovements();
void testRotation();
void testComprehensiveSequence();
void printMovementStatus();
void runTestSequence();

void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            // Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            ControllerProperties properties = ctl->getProperties();
            // Serial.printf("Controller model: %s\n", ctl->getModelName().c_str());
            myControllers[i] = ctl;
            foundEmptySlot = true;
            
            // Exit test mode when controller connects
            testMode = false;
            Serial.println("Controller connected. Exiting test mode.");
            
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not find empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            // Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }
    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
    
    // When controller disconnects, stop all motion
    motionControl.stop();
}

float normalizeThumbstickValue(int value) {
    float normalized = value / 512.0f;
    
    if (normalized > 1.0f) normalized = 1.0f;
    if (normalized < -1.0f) normalized = -1.0f;
    
    return normalized;
}

void processGamepad(ControllerPtr ctl) {
    // Check if Y button is pressed to enter test mode
    if (ctl->y()) {
        Serial.println("Y button pressed! Entering test mode");
        testMode = true;
        return;
    }
    
    float xComponent = normalizeThumbstickValue(ctl->axisX());
    float yComponent = -normalizeThumbstickValue(ctl->axisY());
    
    if (ctl->x()) {
        // X button is pressed - use it for rotation
        Serial.println("X button pressed! Rotating clockwise");
        motionControl.rotate(true);
    } else {
        motionControl.moveVector(xComponent, yComponent);
    }
    
    // Print debug information
    // Serial.printf(
    //     "idx=%d, X: %s, Vector: (%.2f, %.2f), Raw: (%d, %d)\n",
    //     ctl->index(),
    //     ctl->x() ? "PRESSED" : "released",
    //     xComponent, 
    //     yComponent,
    //     ctl->axisX(),
    //     ctl->axisY()
    // );
}

void processControllers() {
    for (auto myController : myControllers) {
        if (myController && myController->isConnected() && myController->hasData()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            }
        }
    }
}

void bleInit() {
    // Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    // Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    // Serial.println("Robot Car Controller Starting...");

    motionControl.begin();
    
    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.forgetBluetoothKeys();
    BP32.enableVirtualDevice(false);
    
    // Enter test mode if no controller is connected on startup
    // Wait 5 seconds for a controller to connect, otherwise run tests
    // Serial.println("Waiting 5 seconds for controller connection...");
    int timeout = 50; // 5 seconds (50 * 100ms)
    bool controllerConnected = false;
    
    while (timeout > 0) {
        BP32.update();
        for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
            if (myControllers[i] != nullptr) {
                controllerConnected = true;
                break;
            }
        }
        
        if (controllerConnected) break;
        
        delay(100);
        timeout--;
    }
    
    if (!controllerConnected) {
        // Serial.println("No controller connected. Entering test mode.");
        testMode = true;
    }
}

void bleUpdate() {
    // Update controller state
    bool dataUpdated = BP32.update();
    if (dataUpdated) {
        processControllers();
    }
    
    // Run test sequence if in test mode
    if (testMode) {
        runTestSequence();
        testMode = false; // Only run the test sequence once
    }
    
    delay(10);
}

// Function to run the complete test sequence once
void runTestSequence() {
    Serial.println("\n=== STARTING TEST SEQUENCE ===");
    delay(3000);  // Give some time to prepare
    
    testBasicMovements();
    testDiagonalMovements();
    testRotation();
    testComprehensiveSequence();
    
    Serial.println("\n=== TEST SEQUENCE COMPLETE ===");
    Serial.println("Press Y button on controller to run test sequence again.");
}

void testBasicMovements() {
    Serial.println("\n=== TESTING BASIC MOVEMENTS ===");
    
    // Forward
    Serial.println("Moving FORWARD...");
    motionControl.moveVector(0.0, 1.0);  // Full forward
    printMovementStatus();
    delay(MOVE_DURATION);
    
    // Stop
    Serial.println("STOPPING...");
    motionControl.stop();
    printMovementStatus();
    delay(PAUSE_DURATION);
    
    // Backward
    Serial.println("Moving BACKWARD...");
    motionControl.moveVector(0.0, -1.0);  // Full backward
    printMovementStatus();
    delay(MOVE_DURATION);
    
    // Stop
    Serial.println("STOPPING...");
    motionControl.stop();
    printMovementStatus();
    delay(PAUSE_DURATION);
    
    // Left
    Serial.println("Moving LEFT...");
    motionControl.moveVector(-1.0, 0.0);  // Full left
    printMovementStatus();
    delay(MOVE_DURATION);
    
    // Stop
    Serial.println("STOPPING...");
    motionControl.stop();
    printMovementStatus();
    delay(PAUSE_DURATION);
    
    // Right
    Serial.println("Moving RIGHT...");
    motionControl.moveVector(1.0, 0.0);  // Full right
    printMovementStatus();
    delay(MOVE_DURATION);
    
    // Stop
    Serial.println("STOPPING...");
    motionControl.stop();
    printMovementStatus();
    delay(PAUSE_DURATION);
}

void testDiagonalMovements() {
    Serial.println("\n=== TESTING DIAGONAL MOVEMENTS ===");
    
    // Forward-Right
    Serial.println("Moving FORWARD-RIGHT...");
    motionControl.moveVector(0.7, 0.7);
    printMovementStatus();
    delay(MOVE_DURATION);
    
    // Stop
    Serial.println("STOPPING...");
    motionControl.stop();
    printMovementStatus();
    delay(PAUSE_DURATION);
    
    // Forward-Left
    Serial.println("Moving FORWARD-LEFT...");
    motionControl.moveVector(-0.7, 0.7);
    printMovementStatus();
    delay(MOVE_DURATION);
    
    // Stop
    Serial.println("STOPPING...");
    motionControl.stop();
    printMovementStatus();
    delay(PAUSE_DURATION);
    
    // Backward-Right
    Serial.println("Moving BACKWARD-RIGHT...");
    motionControl.moveVector(0.7, -0.7);
    printMovementStatus();
    delay(MOVE_DURATION);
    
    // Stop
    Serial.println("STOPPING...");
    motionControl.stop();
    printMovementStatus();
    delay(PAUSE_DURATION);
    
    // Backward-Left
    Serial.println("Moving BACKWARD-LEFT...");
    motionControl.moveVector(-0.7, -0.7);
    printMovementStatus();
    delay(MOVE_DURATION);
    
    // Stop
    Serial.println("STOPPING...");
    motionControl.stop();
    printMovementStatus();
    delay(PAUSE_DURATION);
}

void testRotation() {
    Serial.println("\n=== TESTING ROTATION ===");
    
    // Clockwise rotation
    Serial.println("Rotating CLOCKWISE...");
    motionControl.rotate(true);
    printMovementStatus();
    delay(MOVE_DURATION);
    
    // Stop
    Serial.println("STOPPING...");
    motionControl.stop();
    printMovementStatus();
    delay(PAUSE_DURATION);
    
    // Counter-clockwise rotation
    Serial.println("Rotating COUNTER-CLOCKWISE...");
    motionControl.rotate(false);
    printMovementStatus();
    delay(MOVE_DURATION);
    
    // Stop
    Serial.println("STOPPING...");
    motionControl.stop();
    printMovementStatus();
    delay(PAUSE_DURATION);
}

void testComprehensiveSequence() {
    Serial.println("\n=== TESTING COMPREHENSIVE SEQUENCE ===");
    
    // Square pattern
    Serial.println("Executing SQUARE PATTERN...");
    
    // Forward
    Serial.println("Moving FORWARD...");
    motionControl.moveVector(0.0, 1.0);
    delay(MOVE_DURATION);
    
    // Right
    Serial.println("Moving RIGHT...");
    motionControl.moveVector(1.0, 0.0);
    delay(MOVE_DURATION);
    
    // Backward
    Serial.println("Moving BACKWARD...");
    motionControl.moveVector(0.0, -1.0);
    delay(MOVE_DURATION);
    
    // Left
    Serial.println("Moving LEFT...");
    motionControl.moveVector(-1.0, 0.0);
    delay(MOVE_DURATION);
    
    // Stop
    Serial.println("STOPPING...");
    motionControl.stop();
    delay(PAUSE_DURATION);
    
    // Spin and move
    Serial.println("Executing SPIN-AND-MOVE PATTERN...");
    
    // Clockwise rotation
    Serial.println("Rotating CLOCKWISE...");
    motionControl.rotate(true);
    delay(MOVE_DURATION / 2);
    
    // Forward
    Serial.println("Moving FORWARD...");
    motionControl.moveVector(0.0, 1.0);
    delay(MOVE_DURATION);
    
    // Counter-clockwise rotation
    Serial.println("Rotating COUNTER-CLOCKWISE...");
    motionControl.rotate(false);
    delay(MOVE_DURATION / 2);
    
    // Backward
    Serial.println("Moving BACKWARD...");
    motionControl.moveVector(0.0, -1.0);
    delay(MOVE_DURATION);
    
    // Stop
    Serial.println("STOPPING...");
    motionControl.stop();
    delay(PAUSE_DURATION);
}

void printMovementStatus() {
    // Serial.printf("Direction: X=%.2f, Y=%.2f, Active=%s\n", 
    //               motionControl.getDirectionX(), 
    //               motionControl.getDirectionY(),
    //               motionControl.isActive() ? "YES" : "NO");
}