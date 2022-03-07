#include "SocketIoTHandler.h"
#include <ESP8266WiFi.h>

void SocketIoTPass() {}

SocketIoTConnected() __attribute__((weak, alias("SocketIoTPass")));

SocketIoTDisconnected() __attribute__((weak, alias("SocketIoTPass")));

SocketIoTOTA() __attribute__((weak, alias("SocketIoTPass")));