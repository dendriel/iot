#include <WiFi.h>

#include "camera.h"
#include "globals.h"
#include "http_server.h"
#include "framebuffer.h"
#include "detector.h"
/**
 * secrets.h file is gitignored. Create it with the following:
* #pragma once
 *
 * #define WIFI_SSID "MyWifiName"
 * #define WIFI_PASSWORD "MyWifiPassword"
 */
#include "secrets.h"

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

const unsigned long captureInterval = 1000;

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