#include "framebuffer.h"

#include <Arduino.h>
#include <esp_heap_caps.h>
#include <string.h>

static uint16_t *frontBuffer = nullptr;
static uint16_t *backBuffer = nullptr;

static size_t frameSize = 0;

bool initFrameBuffers(size_t width, size_t height)
{
    frameSize = width * height * sizeof(uint16_t);

    frontBuffer = (uint16_t *)heap_caps_malloc(frameSize, MALLOC_CAP_SPIRAM);
    backBuffer  = (uint16_t *)heap_caps_malloc(frameSize, MALLOC_CAP_SPIRAM);

    if (frontBuffer == nullptr || backBuffer == nullptr)
    {
        Serial.println("Unable to allocate frame buffers.");
        return false;
    }

    memset(frontBuffer, 0, frameSize);
    memset(backBuffer, 0, frameSize);

    Serial.printf("Frame size: %u bytes\n", frameSize);
    Serial.println("Double buffer initialized.");

    return true;
}

uint16_t *getBackBuffer()
{
    return backBuffer;
}

const uint16_t *getFrontBuffer()
{
    return frontBuffer;
}

void publishBuffer()
{
    uint16_t *tmp = frontBuffer;
    frontBuffer = backBuffer;
    backBuffer = tmp;

    Serial.print("PIXEL[0]: ");
    Serial.println(backBuffer[0]);

    Serial.print("PIXEL[1000]: ");
    Serial.println(backBuffer[1000]);
}

size_t getFrameSize()
{
    return frameSize;
}