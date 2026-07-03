#include "detector.h"
#include "framebuffer.h"
#include <Arduino.h>

static const int IMAGE_WIDTH  = 320;
static const int IMAGE_HEIGHT = 240;

static bool gTargetDetected = false;

// ---------------- CONFIG ----------------
#define EDGE_THRESHOLD 28
#define MIN_LINE_RATIO 0.70   // 70% da linha precisa ser borda
#define STEP_SCAN      2
#define SIZE_STEP      6
#define MIN_SIZE       20
// ----------------------------------------

// RGB converter
static inline void rgb565ToRgb(uint16_t pixel, uint8_t &r, uint8_t &g, uint8_t &b)
{
    r = ((pixel >> 11) & 0x1F) << 3;
    g = ((pixel >> 5) & 0x3F) << 2;
    b = (pixel & 0x1F) << 3;
}

// Luma (grayscale)
static inline uint8_t getLuma(uint16_t pixel)
{
    uint8_t r, g, b;
    rgb565ToRgb(pixel, r, g, b);
    return (uint8_t)((30 * r + 59 * g + 11 * b) / 100);
}

// Edge detection (local gradient)
static inline bool isEdge(uint8_t a, uint8_t b)
{
    return abs(a - b) > EDGE_THRESHOLD;
}

// ---------------- EDGE MAP ----------------
static void buildEdgeMap(uint8_t *edgeMap, const uint16_t *img, int w, int h)
{
    for (int y = 1; y < h - 1; y++)
    {
        for (int x = 1; x < w - 1; x++)
        {
            uint8_t c  = getLuma(img[y * w + x]);
            uint8_t r  = getLuma(img[y * w + x + 1]);
            uint8_t d  = getLuma(img[(y + 1) * w + x]);

            edgeMap[y * w + x] = (isEdge(c, r) || isEdge(c, d)) ? 1 : 0;
        }
    }
}

// ---------------- LINE VALIDATION ----------------
static bool checkHorizontal(const uint8_t *edgeMap, int w, int x0, int x1, int y)
{
    int hits = 0;
    int len  = x1 - x0;

    for (int x = x0; x <= x1; x++)
    {
        if (edgeMap[y * w + x]) hits++;
    }

    return hits > len * MIN_LINE_RATIO;
}

static bool checkVertical(const uint8_t *edgeMap, int w, int y0, int y1, int x)
{
    int hits = 0;
    int len  = y1 - y0;

    for (int y = y0; y <= y1; y++)
    {
        if (edgeMap[y * w + x]) hits++;
    }

    return hits > len * MIN_LINE_RATIO;
}

// ---------------- MAIN DETECTOR ----------------
static bool detectSquareRing(const uint16_t *img, int w, int h)
{
    static uint8_t edgeMap[320 * 240]; // memória fixa (ok p/ ESP32)

    buildEdgeMap(edgeMap, img, w, h);

    for (int size = MIN_SIZE; size < (w < h ? w : h); size += SIZE_STEP)
    {
        for (int y = 0; y < h - size; y += STEP_SCAN)
        {
            for (int x = 0; x < w - size; x += STEP_SCAN)
            {
                int x2 = x + size;
                int y2 = y + size;

                // TOP + BOTTOM
                if (!checkHorizontal(edgeMap, w, x, x2, y))  continue;
                if (!checkHorizontal(edgeMap, w, x, x2, y2)) continue;

                // LEFT + RIGHT
                if (!checkVertical(edgeMap, w, y, y2, x))  continue;
                if (!checkVertical(edgeMap, w, y, y2, x2)) continue;

                // encontrou um aro/quadrado fechado
                return true;
            }
        }
    }

    return false;
}

// ---------------- PUBLIC API ----------------
bool detectTarget(const uint16_t *frame)
{
    if (!frame)
    {
        gTargetDetected = false;
        return false;
    }

    gTargetDetected = detectSquareRing(frame, IMAGE_WIDTH, IMAGE_HEIGHT);
    return gTargetDetected;
}

bool isTargetDetected()
{
    return gTargetDetected;
}