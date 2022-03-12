#include "SocketIoTHandler.h"

void SocketIoTPass() {}
void SocketIoTOTAPass(const char*) {}

SocketIoTConnected() __attribute__((weak, alias("SocketIoTPass")));

SocketIoTDisconnected() __attribute__((weak, alias("SocketIoTPass")));

SocketIoTOTA() __attribute__((weak, alias("SocketIoTOTAPass")));