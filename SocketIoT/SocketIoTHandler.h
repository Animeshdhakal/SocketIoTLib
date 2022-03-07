#ifndef SocketIoTHandler_h
#define SocketIoTHandler_h

#include "SocketIoTData.h"

#define SocketIoTWrite(PIN)                                                     \
    void SocketIoT_Write_##PIN(SocketIoTData &);                                \
    uint8_t tempW##PIN = registerSocketIOTCallback(PIN, SocketIoT_Write_##PIN); \
    void SocketIoT_Write_##PIN(SocketIoTData &data)

#define SocketIoT_Write(PIN) \
    SocketIOTWrite(PIN)

#define SocketIoTConnected() void socketIoTConnected()
#define SocketIoTDisconnected() void socketIoTDisconnected()
#define SocketIoTOTA() void socketIoTOTA(const char* url)

typedef void (*SocketIoTCallback)(SocketIoTData &data);

static SocketIoTCallback SocketIoTHandlers[20] = {NULL};

static uint8_t registerSocketIOTCallback(uint8_t pin, SocketIoTCallback cb)
{
    SocketIoTHandlers[pin] = cb;
    return 0;
}

#ifdef __cplusplus
extern "C"
{
#endif

    SocketIoTConnected();

    SocketIoTDisconnected();

    SocketIoTOTA();

    void SocketIoTPass();

#ifdef __cplusplus
}
#endif

#endif