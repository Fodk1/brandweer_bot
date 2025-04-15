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

### image_processing.c

The [image_processing.c](src/image_processing.c) module runs on the M4 core and is responsible for locating fires in an thermal image, this data can be used by other parts of the robot to aim at these fires. So they can be extinguished.

**API**

The API exposes the function `processImage()` which takes a *32x24* array of bytes representing a thermal image, the function will return a struct containing the amount of objects found togheter with all found objects.

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

### thermal_cam.cpp

the [thermal_cam.cpp](src\thermal_cam.cpp) module is used to cummunicate with the [MLX90640](https://www.melexis.com/en/documents/documentation/datasheets/datasheet-mlx90640) (thermal camera) via *I2C*.

**API**

The API exposes two functions:
- `thermalCamInit()` which can be used to initialise the thermal camera and make it ready for communication, this function should be called in the setup of the system.
- `getFrame()` which can be used to get a frame from the camera, timing for getting the frame is handled by the function itself.

**usage**

To get a frame the `getFrame()` function can simply be called passing a 2D buffer pointer as the first argument, the function will wait for the camera
to produce a full frame which can take up to 62.5 ms depending on when the function was called.

```cpp
uint16_t frame[IMAGE_HEIGHT][IMAGE_WIDTH];
getFrame(frame); // Fill 2D frame buffer with data
```