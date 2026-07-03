#ifndef CAMERA_H
#define CAMERA_H

#include <Arduino.h>
#include "esp_camera.h"

bool initCamera();
bool captureFrame();

uint16_t getFrameWidth();
uint16_t getFrameHeight();

#endif