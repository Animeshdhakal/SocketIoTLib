#include "SocketIoT.h"

template <typename T>
SocketIoT<T>::SocketIoT(T &client) : client(client), last_recv(0), last_ping(0), last_send(0), state(CONNECTING) {}

template <typename T>
void SocketIoT<T>::init(const char *token, const char *host, uint16_t port)
{
    this->token = token;
    this->host = host;
    this->port = port;

    if (client.connect(host, port))
    {
        state = CONNECTING;
        authenticate();
    }
}

template <typename T>
void SocketIoT<T>::send(uint8_t *msg, size_t len)
{
    if (client.connected())
    {
        client.write(msg, len);
        last_send = Millis();
    }
}

template <typename T>
void SocketIoT<T>::authenticate()
{
    SocketIoTHeader hdr;
    hdr.msg_len = htons(strlen(this->token));
    hdr.msg_type = htons(AUTH);
    uint8_t buff[sizeof(SocketIoTHeader) + strlen(this->token)];
    memcpy(buff, &hdr, sizeof(SocketIoTHeader));
    memcpy(buff + sizeof(SocketIoTHeader), this->token, strlen(this->token));
    this->send(buff, sizeof(buff));
}

template <typename T>
void SocketIoT<T>::loop()
{
    YIELD();

    if (client.connected())
    {
        while (client.available() > 0)
        {
            SocketIoTHeader hdr;
            int rlen = client.read((uint8_t *)&hdr, sizeof(SocketIoTHeader));
            if (rlen == sizeof(SocketIoTHeader))
            {
                last_recv = Millis();
                hdr.msg_len = ntohs(hdr.msg_len);
                hdr.msg_type = ntohs(hdr.msg_type);

                char buff[hdr.msg_len + 1];
                client.read((uint8_t *)buff, hdr.msg_len);
                buff[hdr.msg_len] = 0;

                switch (hdr.msg_type)
                {
                case AUTH:
                    if (buff[0] - '0' == 1)
                    {
                        Serial.println("Authenticated");
                        state = CONNECTED;
                    }
                    else
                    {
                        Serial.println("Authentication failed");
                        state = CONNECTING;
                    }
                    break;
                case WRITE:
                    Serial.println(buff);
                    break;
                }
            }
        }
    }

    if (state == CONNECTED)
    {
        time_millis_t now = Millis();

        if ((now - last_recv) > 2 * HEARTBEAT)
        {
            Serial.println("SocketIoT: lost connection");

            state = CONNECTING;

            last_ping = 0;
            last_recv = 0;
            last_send = 0;

            client.stop();
        }

        else if ((now - last_ping) > HEARTBEAT && ((now - last_recv) > HEARTBEAT || (now - last_send) > HEARTBEAT))
        {
            Serial.println("SocketIoT: ping");
            ping();
            last_ping = now;
        }
    }
    else
    {
        if (!client.connected())
        {
            if (client.connect(host, port))
            {
                Serial.println("SocketIoT: connected");
                state = CONNECTING;
                authenticate();
            }
        }
    }
}

template <typename T>
void SocketIoT<T>::ping()
{
    SocketIoTHeader hdr;
    hdr.msg_len = htons(0);
    hdr.msg_type = htons(PING);
    client.write((uint8_t *)&hdr, sizeof(SocketIoTHeader));
}

template <typename T>
void SocketIoT<T>::reconnect()
{
    state = CONNECTING;
    client.stop();
}

WiFiClient client;
SocketIoT socketIoT(client);