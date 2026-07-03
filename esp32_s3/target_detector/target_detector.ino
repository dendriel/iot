#include <WiFi.h>

#include "camera.h"
#include "globals.h"
#include "webserver.h"
#include "framebuffer.h"
#include "detector.h"

const char *ssid = "";
const char *password = "";

const unsigned long captureInterval = 500;

unsigned long lastCapture = 0;

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    if (!initCamera())
    {
        while (true)
            delay(1000);
    }

    WiFi.begin(ssid, password);

    Serial.print("Connecting");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    Serial.print("IP Address: ");

    Serial.println(WiFi.localIP());

    startWebServer();
}

void loop()
{
    handleWebServer();

    if (millis() - lastCapture >= captureInterval)
    {
        lastCapture = millis();

        if (captureFrame())
        {
            gTargetPresent = detectTarget(getFrontBuffer());

            Serial.println(gTargetPresent ? "TARGET FOUND" : "TARGET NOT FOUND");
        }
    }
}