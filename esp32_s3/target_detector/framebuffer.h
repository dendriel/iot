#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <Arduino.h>

bool initFrameBuffers(size_t width, size_t height);

uint16_t *getBackBuffer();

const uint16_t *getFrontBuffer();

void publishBuffer();

size_t getFrameSize();

#endif