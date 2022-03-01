#ifndef SocketIoTClient_h
#define SocketIoTClient_h

#include "SocketIoTData.h"
#include "SocketIoTDebug.h"
#include "SocketIoTSpec.h"
#include "SocketIoTConfig.h"
#include "SocketIoTHandler.h"

template <typename Client>
class SocketIoTClient
{
protected:
    Client &client;
    const char *auth;
    const char *host;
    uint16_t port;
    SocketIoTState state;
    time_millis_t last_ping;
    time_millis_t last_recv;
    time_millis_t last_send;

public:
    SocketIoTClient(Client &client) : client(client), last_ping(0), last_send(0), last_recv(0), state(CONNECTING) {}

    void init(const char *auth, const char *host, uint16_t port)
    {
        this->auth = auth;
        this->host = host;
        this->port = port;
        LOG4(SF("Connecting to "), host, ":", port);
    }

    void processWrite(SocketIoTData data)
    {
        const uint8_t pin = data.toInt();
        if (++data >= data.end())
        {
            return;
        }
        if (SocketIoTHandlers[pin])
        {
            SocketIoTHandlers[pin](data);
        }
        else
        {
            LOG2(SF("No handler for pin "), pin);
        }
    }

    template <typename T>
    void write(uint8_t pin, T value)
    {
        char buff[100];
        SocketIoTData data(buff, 0, sizeof(buff));
        data.put(pin);
        data.put(value);
        sendMsg(WRITE, data.toString(), data.length());
    }

    void processData()
    {
        SocketIoTHeader hdr;
        if (client.read((uint8_t *)&hdr, sizeof(hdr)) == sizeof(SocketIoTHeader))
        {
            last_recv = MILLIS();
            hdr.msg_type = ntohs(hdr.msg_type);
            hdr.msg_len = ntohs(hdr.msg_len);

            char buff[hdr.msg_len + 1];
            client.read((uint8_t *)buff, hdr.msg_len);
            buff[hdr.msg_len] = 0;

            SocketIoTData data(buff, hdr.msg_len);

            switch (hdr.msg_type)
            {
            case AUTH:

                if (buff[0] - '0' == 1)
                {
                    LOG1(SF("Authenticated"));
                    sendInfo();
                    state = CONNECTED;
                    socketIoTConnected();
                }
                else
                {
                    LOG1(SF("Authentication failed"));
                    state = AUTH_FAILED;
                }
                break;
            case PING:
                LOG1(SF("PING"));
                break;
            case WRITE:
                processWrite(data);
                break;
            }
        }
    }

    void sendInfo()
    {
        static const char info[] SPROGMEM = "info\0" InfoParam("hbeat", NumToString(HEARTBEAT)) InfoParam("build", __DATE__ " " __TIME__) "\0";
        size_t actualsize = sizeof(info) - 5 - 2;
        sendMsg(INFO, info + 5, actualsize);
    }

    void authenticate()
    {
        sendMsg(AUTH, auth, strlen(auth));
    }

    void sendMsg(uint16_t msg_type, const char *msg = NULL, size_t length = 0)
    {
        SocketIoTHeader hdr = {length, msg_type};
        hdr.msg_len = htons(hdr.msg_len);
        hdr.msg_type = htons(hdr.msg_type);
        uint8_t fullbuff[length + sizeof(SocketIoTHeader)];
        memcpy(fullbuff, &hdr, sizeof(SocketIoTHeader));
        memcpy(fullbuff + sizeof(SocketIoTHeader), msg, length);
        client.write(fullbuff, length + sizeof(SocketIoTHeader));
        last_send = MILLIS();
    }

    bool connected()
    {
        return state == CONNECTED;
    }

    void syncWithServer()
    {
        sendMsg(SYNC);
    }

    void loop()
    {
        YIELD();

        if (client.connected())
        {
            while (client.available() > 0)
            {
                processData();
            }
        }

        if (state == CONNECTED)
        {
            time_millis_t now = MILLIS();

            if (now - last_recv > HEARTBEAT * 1000 * 2)
            {
                LOG1(SF("Server Disconnected"));
                state = CONNECTING;
                socketIoTDisconnected();
            }
            else if (now - last_ping > HEARTBEAT * 1000 && (now - last_recv > HEARTBEAT * 1000 || now - last_send > HEARTBEAT * 1000))
            {
                LOG1(SF("Sending Ping"));
                sendMsg(PING);
                last_ping = now;
            }
        }
        else if (state == CONNECTING)
        {
            if (!client.connected() && client.connect(host, port))
            {
                LOG1(SF("Connected to Server"));
                authenticate();
            }
        }
    }
};

#endif