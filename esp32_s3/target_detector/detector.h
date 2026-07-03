#ifndef DETECTOR_H
#define DETECTOR_H

#include <stdint.h>

bool detectTarget(const uint16_t *frame);
bool isTargetDetected();

#endif