#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "SocketIoT.h"

void setup()
{

    Serial.begin(115200);
    WiFi.begin("SSID", "PASSWORD");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("Connected to WiFi");

    socketIoT.init("TOKEN", "HOST", 443);
}

void loop()
{
    socketIoT.loop();
}
