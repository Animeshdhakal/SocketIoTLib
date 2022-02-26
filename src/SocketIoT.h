#ifndef SOCKETIOT_H
#define SOCKETIOT_H

#include <stdlib.h>
#include <ESP8266WiFi.h>

typedef uint16_t time_millis_t;

time_millis_t Millis()
{
    return millis();
}

#define YIELD yield

#define HEARTBEAT 10000

struct SocketIoTHeader
{
    uint16_t msg_len;
    uint16_t msg_type;
};

enum State
{
    CONNECTED,
    CONNECTING,
};

enum MsgType
{
    _,
    AUTH,
    WRITE,
    READ,
    PING,
    SYNC,
    INFO
};

template <typename Client>
class SocketIoT
{
    Client &client;
    time_millis_t last_recv;
    time_millis_t last_send;
    time_millis_t last_ping;
    const char *token;
    const char *host;
    uint16_t port;
    State state;

public:
    SocketIoT(Client &);

    void init(const char *, const char *, uint16_t);

    void send(uint8_t *, size_t);

    void authenticate();

    void ping();

    void reconnect();

    void loop();
};

extern SocketIoT<WiFiClient> socketIoT;

#endif