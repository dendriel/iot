#include "detector.h"

#include <Arduino.h>

static const int IMAGE_WIDTH  = 320;
static const int IMAGE_HEIGHT = 240;

static bool gTargetDetected = false;

#define GREEN_THRESHOLD   140
#define RED_THRESHOLD     150
#define BLUE_THRESHOLD    90

#define MIN_HP_WIDTH      40
#define MONSTER_HEIGHT    20

#define ICON_WIDTH        22
#define ICON_HEIGHT       22

#define BORDER_MARGIN     2

//------------------------------------------------------------

struct Monster
{
    int iconX;
    int iconY;
};

//------------------------------------------------------------

static inline void rgb565ToRgb(uint16_t pixel,
                               uint8_t &r,
                               uint8_t &g,
                               uint8_t &b)
{
    r = ((pixel >> 11) & 0x1F) << 3;
    g = ((pixel >> 5) & 0x3F) << 2;
    b = (pixel & 0x1F) << 3;
}

//------------------------------------------------------------

static inline bool isGreen(uint16_t pixel)
{
    uint8_t r,g,b;

    rgb565ToRgb(pixel,r,g,b);

    return
        g > GREEN_THRESHOLD &&
        r < 100 &&
        b < 100;
}

//------------------------------------------------------------

static inline bool isPink(uint16_t pixel)
{
    uint8_t r,g,b;

    rgb565ToRgb(pixel,r,g,b);

    return
        r > RED_THRESHOLD &&
        g < 110 &&
        b > BLUE_THRESHOLD;
}

//------------------------------------------------------------
// Procura barras verdes
//------------------------------------------------------------

static int findMonsters(const uint16_t *img,
                        Monster *monsters,
                        int maxMonsters)
{
    int count = 0;

    for(int y=10; y<220; y++)
    {
        int run = 0;

        for(int x=90; x<310; x++)
        {
            if(isGreen(img[y*IMAGE_WIDTH+x]))
                run++;
            else
                run=0;

            if(run>MIN_HP_WIDTH)
            {
                if(count<maxMonsters)
                {
                    monsters[count].iconX = x-run-18;
                    monsters[count].iconY = y-12;

                    count++;
                }

                y += MONSTER_HEIGHT;

                break;
            }
        }
    }

    return count;
}

//------------------------------------------------------------

static bool hasPinkBorder(const uint16_t *img,
                          int x,
                          int y)
{
    int hits=0;
    int total=0;

    //--------------------------------------------------------
    // topo
    //--------------------------------------------------------

    for(int i=0;i<ICON_WIDTH;i++)
    {
        total++;

        if(isPink(img[(y+BORDER_MARGIN)*IMAGE_WIDTH+
                      (x+i)]))
            hits++;
    }

    //--------------------------------------------------------
    // baixo
    //--------------------------------------------------------

    for(int i=0;i<ICON_WIDTH;i++)
    {
        total++;

        if(isPink(img[(y+ICON_HEIGHT-BORDER_MARGIN)*IMAGE_WIDTH+
                      (x+i)]))
            hits++;
    }

    //--------------------------------------------------------
    // esquerda
    //--------------------------------------------------------

    for(int i=0;i<ICON_HEIGHT;i++)
    {
        total++;

        if(isPink(img[(y+i)*IMAGE_WIDTH+
                      (x+BORDER_MARGIN)]))
            hits++;
    }

    //--------------------------------------------------------
    // direita
    //--------------------------------------------------------

    for(int i=0;i<ICON_HEIGHT;i++)
    {
        total++;

        if(isPink(img[(y+i)*IMAGE_WIDTH+
                      (x+ICON_WIDTH-BORDER_MARGIN)]))
            hits++;
    }

    return hits > total*0.55;
}

//------------------------------------------------------------

bool detectTarget(const uint16_t *frame)
{
    if(frame==nullptr)
    {
        gTargetDetected=false;
        return false;
    }

    Monster monsters[12];

    int total=findMonsters(frame,
                           monsters,
                           12);

    for(int i=0;i<total;i++)
    {
        if(hasPinkBorder(frame,
                         monsters[i].iconX,
                         monsters[i].iconY))
        {
            gTargetDetected=true;
            return true;
        }
    }

    gTargetDetected=false;

    return false;
}

//------------------------------------------------------------

bool isTargetDetected()
{
    return gTargetDetected;
}