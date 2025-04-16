#ifndef FLAGS_H
#define FLAGS_H

#include "rtos.h"

#define START_SCAN_FLAG 0B01
#define START_TRACK_FLAG 0B10
#define NEW_FRAME_FLAG 0B100
#define MANUAL_CONTROL_FLAG 0B1000

rtos::EventFlags flags;

#endif