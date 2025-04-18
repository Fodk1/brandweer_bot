# Brandweer_bot

The purpose of the Brandweer Bot *(BB)* is to automatically extinguish fires around it, to do so it will need functionality for detecting, tracking and moving closer to fires.

## Microcontroller

The microcontroller used for this project is the [Portenta h7](https://docs.arduino.cc/hardware/portenta-h7/), this board contains two cores:
- *M7* core
- *M4* core

In this project, the *M7* core will be used for general purpose tasks while the *M4* core will only be used for image processing because this can be quite taxing on the CPU.

## Modules

### main.cpp

The program will be uploaded to both cores, the [main.cpp](src/main.cpp) module will determine what part wil be used depending on which core it is running.

### imageProcessing.c

The [imageProcessing.c](src/imageProcessing.c) module runs on the M4 core and is responsible for locating fires in an thermal image, this data can be used by other parts of the robot to aim at these fires. So they can be extinguished.

**API**

The API exposes the function `processImage()` which takes a *32x24* array of bytes representing a thermal image, the function will return a struct containing the amount of objects found together with all found objects.

**usage**

```c
AllPerceptedObjs allObjs = processImage(testImage); // Process an image
        
for (uint8_t i = 0; i < allObjs.objCount; i++) { // Iterate over found objects
    PerceptedObj obj = allObjs.objs[i];

    float y = obj.y;                // Y position of object              
    float x = obj.x;                // X position of object
    uint16_t size = obj.obj_size;   // The amount of pixels an object takes up
}
free(allObjs.obj); // Make sure to free the memory after use
```

### thermalCam.cpp

the [thermalCam.cpp](src\thermalCam.cpp) module is used to communicate with the [MLX90640](https://www.melexis.com/en/documents/documentation/datasheets/datasheet-mlx90640) (thermal camera) via *I2C*.

**API**

The API exposes two functions:
- `thermalCamInit()` which can be used to initialize the thermal camera and make it ready for communication, this function should be called in the setup of the system.
- `getFrame()` which can be used to get a frame from the camera, timing for getting the frame is handled by the function itself.

**usage**

To get a frame the `getFrame()` function can simply be called passing a 2D buffer pointer as the first argument, the function will wait for the camera
to produce a full frame which can take up to 62.5 ms depending on when the function was called.

```cpp
uint16_t frame[IMAGE_HEIGHT][IMAGE_WIDTH];
getFrame(frame); // Fill 2D frame buffer with data
```

### gyro.cpp

The [gyro.cpp](src\gyro.cpp) module is used to communicate with the [MPU6050](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf) (gyroscope) via *I2C*.

**API**

The API exposes three functions:
- `gyroInit()` which can be used to initialize the gyroscope and make it ready for communication, this function should be called in the setup of the system.
- `gyroUpdate()` which can be used to update the current orientation of the gyroscope, this function should be called repeatedly to retain accurate measurements.
- `getXAxis()` which returns the current orientation of the turret.

### BLE.cpp

The [BLE.cpp](src\BLE.cpp) module is used to host a bluetooth low power acces point to allow other devices to interface with the portenta H7.

**API**

The API exposes two functions:
- `BLEInit()` which can be used to initialize the bluetooth acces point and make it ready for communication, this function should be called in the setup of the system.
- `BLEUpdate()` which polls for BLE connections and allows for communication, this function should be called repeatedly to allow proper BLE connectivity.

### timerInterrupt.cpp

The [timerInterrupt.cpp](src\timerInterrupt.cpp) module is start a timer that calls a interrupt after a given time.

**API**

The API exposes the function `startTimer()`, which takes an unsigned integer as milliseconds between interrupts, and a callback function that get called after the given milliseconds.

**usage**
```cpp
void callbackFunction(); // Define callback function
...

startTimer(1000, callbackFunction); // Callback function get called every 1000 ms
```

### watchdog.cpp

The [watchdog.cpp](src\watchdog.cpp) module is used to start and maintain a watchdog timer.

**API**
The API exposes two functions:
- `startWatchdog()` which starts the watchdog timer, an unsigned integer is given as the maximum feeding time before it resets the system.
- `feedWatchdog()` which resets the timer and allows the system to run until the previously given maximum feeding time runs out. 

### turretMovement.cpp

The [turretMovement.cpp](src\turretMovement.cpp) module is used to rotate the turret on its X and Y axis.

**API**
The API exposes four functions:
- `turretInitXAxis()` and `turretInitYAxis()` which initialize the required pins for the motors, these functions should be called in the setup of the system.
- `turretSetXMovement()` and `turretSetYMovement()` which put the motors in motion.
