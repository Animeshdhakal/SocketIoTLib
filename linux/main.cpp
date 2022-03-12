#include <iostream>

#define FIRMWARE_VERSION "1.0"
#define BLUEPRINT_ID "Animesh"
#define DEBUG

#include "SocketIoTLinuxSSLConnector.h"
#include "SocketIoTClient.h"


static SocketIoTLinuxConnnector connector;
static SocketIoTClient<SocketIoTLinuxConnnector> SocketIoT(connector);

SocketIoTWrite(1){
    LOG1(data.toInt());
}

int main(){
    SocketIoT.init("", "", 443);

    for(;;){
        SocketIoT.loop();
    }

}