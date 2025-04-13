// Het BrandweerBot Team 28-02-2025

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "imageProcessing.h"

#define CRITICAL_TEMP 100
#define NONE -1

/*
 * Strange pattern: width + height - 2 = largest stack usage with floodfill algorithm, why? Idk...
 * TODO: Make mathematical proof?
 */
#define MAX_STACK_SIZE(w, h) ((w) + (h) - 2)

typedef struct {
    uint8_t y;
    uint8_t x;
} PixPos;

void ppsPush(PixPos* stack, uint8_t* sp, PixPos pixPos) {
    stack[(*sp)++] = pixPos;
}

PixPos ppsPop(PixPos* stack, uint8_t* sp) {
    return stack[--(*sp)];
}

/**
 * Checks if there is a hot object at a certain pixel, 
 * if there is one it maps the entire object with the current object ID in posState
 */
void findObj(const uint8_t image[][IMAGE_WIDTH], uint8_t posState[][IMAGE_WIDTH], PixPos startPos, uint8_t* objID) {

    // Only handle pixel if it is on a hot object
    if (image[startPos.y][startPos.x] < CRITICAL_TEMP) return;
    if (posState[startPos.y][startPos.x] != 0) return;
    
    // Map object by using the floodfill algorithm
    PixPos stack[MAX_STACK_SIZE(IMAGE_WIDTH, IMAGE_HEIGHT)] = {};
    uint8_t sp = 0;

    ppsPush(stack, &sp, startPos); // Add startpos as first pixel as begin of the flood
    while (sp > 0) {
        PixPos pos = ppsPop(stack, &sp);

        if (pos.y > IMAGE_WIDTH - 1 || pos.x > IMAGE_WIDTH - 1) continue;   // Pixel is outside of the screen, can't check
        if (image[pos.y][pos.x] < CRITICAL_TEMP) continue;                  // Pixel is not hot, flood ends here
        if (posState[pos.y][pos.x] != 0) continue;                          // Hot object that pixel covers has already been mapped

        posState[pos.y][pos.x] = *objID; // Mark pixel with ID of object

        // Define all adjacent pixels
        PixPos northPixel = {.y = pos.y - 1, .x = pos.x};
        PixPos eastPixel =  {.y = pos.y, .x = pos.x + 1};
        PixPos southPixel = {.y = pos.y + 1, .x = pos.x};
        PixPos westPixel =  {.y = pos.y, .x = pos.x - 1};

        // Add all adjacent pixels
        ppsPush(stack, &sp, northPixel);
        ppsPush(stack, &sp, eastPixel);
        ppsPush(stack, &sp, southPixel);
        ppsPush(stack, &sp, westPixel);
    }
    (*objID)++; // Change ID for next object
}

/**
 * Takes a grid of object ID's and returns the centers as well as the size of the objects in that grid.
 */
PerceivedObj* getObjData(const uint8_t posState[][IMAGE_WIDTH], uint8_t objsInSight) {
    struct DataFind {
        uint16_t combinedY;     // Sum of all Y positions of an object
        uint16_t combinedX;     // Sum of all X positions of an object
        uint16_t totalPixels;   // Count of all pixels in an object
    };
    struct DataFind* objData = (struct DataFind*) malloc(sizeof(struct DataFind) * objsInSight);
    struct DataFind init = {.combinedY = 0, .combinedX = 0, .totalPixels = 0};
    for (uint8_t i = 0; i < objsInSight; i++) // Init all objects
        objData[i] = init;

    // Populate data of objects
    for (uint8_t y = 0; y < IMAGE_HEIGHT; y++) {
        for (uint8_t x = 0; x < IMAGE_WIDTH; x++) {

            uint8_t pixelObjID = posState[y][x];
            if (pixelObjID == 0) continue; // No hot object covers pixel
            
            // Update data of the object at the pixel
            uint8_t objIndex = pixelObjID - 1;
            objData[objIndex].combinedY += y;
            objData[objIndex].combinedX += x;
            objData[objIndex].totalPixels++;
        }
    }
    // Store found object data in 'PerceptedObj' structs
    PerceivedObj* perceivedObj = (PerceivedObj*) malloc(sizeof(PerceivedObj) * objsInSight);
    for (uint8_t i = 0; i < objsInSight; i++) {
        perceivedObj[i].obj_size = objData[i].totalPixels;

        // Calculate average XY position (center of object)
        perceivedObj[i].y = ((float) objData[i].combinedY) / objData[i].totalPixels;
        perceivedObj[i].x = ((float) objData[i].combinedX) / objData[i].totalPixels;
    }
    free(objData);
    return perceivedObj;
}

AllPerceivedObjs processImage(const uint8_t image[][IMAGE_WIDTH]) {
    // Array with the state of each position (if it has already been mapped or not)
    uint8_t posState[IMAGE_HEIGHT][IMAGE_WIDTH] = {};
    uint8_t currObjID = 1;

    // Map all hot objects in view of the camera
    for (uint8_t y = 0; y < IMAGE_HEIGHT; y++) {
        for (uint8_t x = 0; x < IMAGE_WIDTH; x++) {
            
            PixPos pixPos = {.y = y, .x = x};
            findObj(image, posState, pixPos, &currObjID); // Map Obj if it exists
        }
    }
    uint8_t objsInSight = currObjID - 1;
    PerceivedObj* perceivedObjs = getObjData(posState, objsInSight);

    // Package objects for simple use
    AllPerceivedObjs allObjs = {.objs = perceivedObjs, .objCount = objsInSight};
    return allObjs;
}