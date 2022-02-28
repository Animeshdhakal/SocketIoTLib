#ifndef SocketIoTHandler_h
#define SocketIoTHandler_h

#include "SocketIoTData.h"

typedef void (*SocketIoTCallback)(SocketIOTData &data);

static SocketIoTCallback SocketIoTHandlers[20] = {NULL};

uint8_t registerSocketIOTCallback(uint8_t pin, SocketIoTCallback cb)
{
    SocketIoTHandlers[pin] = cb;
    return 0;
}

#define SocketIoTWrite(PIN)                                                     \
    void SocketIOT_Write_##PIN(SocketIOTData &);                                \
    uint8_t tempW##PIN = registerSocketIOTCallback(PIN, SocketIOT_Write_##PIN); \
    void SocketIOT_Write_##PIN(SocketIOTData &data)

#define SocketIoT_Write(PIN) \
    SocketIOTWrite(PIN)

#endif