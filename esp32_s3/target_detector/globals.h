#ifndef GLOBALS_H
#define GLOBALS_H

#include "esp_camera.h"

extern camera_fb_t *currentFrame;

extern bool frameAvailable;
extern bool gTargetPresent;

#endif