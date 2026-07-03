#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "http_server.h"
#include "framebuffer.h"
#include "camera.h"
#include "img_converters.h"
#include "esp_camera.h"
#include "detector.h"

static WebServer server(80);


static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 Camera</title>

<style>
body{
    margin:0;
    background:#000;
    color:white;
    text-align:center;
    font-family:Arial;
}

#status{
    font-size:28px;
    font-weight:bold;
    padding:15px;
}

img{
    width:100%;
    height:auto;
}
</style>

</head>

<body>

<h2>{{STATUS}}</h2>

<img id="img">

<script>

async function update()
{
    const img = document.getElementById("img");
    img.src="/image?t="+Date.now();

    const res = await fetch("/status");
    const txt = await res.text();

    document.getElementById("status").innerHTML = txt;
}

update();
setInterval(update,1000);

</script>

</body>
</html>
)rawliteral";

void handleRoot()
{
    String html = FPSTR(INDEX_HTML);

    html.replace(
        "{{STATUS}}",
        isTargetDetected() ? "TARGET DETECTED" : "TARGET NOT DETECTED");

    server.send(200, "text/html", html);
}

void handleImageOKJPEG()
{
    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb)
    {
        server.send(500, "text/plain", "Camera failed");
        return;
    }

    WiFiClient client = server.client();

    client.printf(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: image/jpeg\r\n"
        "Content-Length: %u\r\n"
        "Connection: close\r\n\r\n",
        fb->len
    );

    client.write(fb->buf, fb->len);

    esp_camera_fb_return(fb);
}


void handleImageOriginal()
{
    const uint16_t *fb = getFrontBuffer();

    if (!fb)
    {
        server.send(500, "text/plain", "No frame");
        return;
    }

    camera_fb_t jpg_fb = {0};

    // converter RGB565 -> JPEG
    // buffer temporário interno da lib
    uint8_t *jpg_buf = nullptr;
    size_t jpg_len = 0;

    bool ok = fmt2jpg(
        (uint8_t *)fb,
        getFrameWidth() * getFrameHeight() * 2, // getFrameSize(),
        getFrameWidth(),
        getFrameHeight(),
        PIXFORMAT_RGB565,
        80,
        &jpg_buf,
        &jpg_len
    );

    if (!ok)
    {
        server.send(500, "text/plain", "JPEG convert failed");
        return;
    }

    server.sendHeader("Content-Type", "image/jpeg");
    server.sendHeader("Content-Length", String(jpg_len));
    server.send(200);

    WiFiClient client = server.client();
    client.write(jpg_buf, jpg_len);

    free(jpg_buf);
}

void handleImage()
{
    const uint16_t *fb = getFrontBuffer();

    if (!fb)
    {
        server.send(503, "text/plain", "No frame");
        return;
    }

    uint8_t *jpg_buf = nullptr;
    size_t jpg_len = 0;

    bool ok = fmt2jpg(
        (uint8_t *)fb,
        getFrameSize(),
        getFrameWidth(),
        getFrameHeight(),
        PIXFORMAT_RGB565,
        80,
        &jpg_buf,
        &jpg_len
    );

    if (!ok)
    {
        server.send(500, "text/plain", "JPEG conversion failed");
        return;
    }

    WiFiClient client = server.client();

    client.printf(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: image/jpeg\r\n"
        "Content-Length: %u\r\n"
        "Connection: close\r\n\r\n",
        jpg_len
    );

    client.write(jpg_buf, jpg_len);

    free(jpg_buf);
}

void startWebServer()
{
    server.on("/", HTTP_GET, handleRoot);
    server.on("/image", HTTP_GET, handleImage);

    server.begin();

    Serial.println("Web server started");
}

void handleWebServer()
{
    server.handleClient();
}

void handleStatus()
{
    if (isTargetDetected())
    {
        server.send(200, "text/plain", "TARGET DETECTED");
    }
    else
    {
        server.send(200, "text/plain", "TARGET NOT DETECTED");
    }
}