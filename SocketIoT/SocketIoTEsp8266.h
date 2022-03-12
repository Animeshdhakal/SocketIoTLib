#ifndef SocketIoTEsp8266_h
#define SocketIoTEsp8266_h
#include <WiFiClient.h>
#include "SocketIoTClient.h"
#include "connectors/SocketIoTArduinoConnector.h"

static WiFiClient wifiClient;
static SocketIoTArduinoConnector<WiFiClient> conn(wifiClient);
static SocketIoTClient<SocketIoTArduinoConnector<WiFiClient>> SocketIoT(conn);


#endif