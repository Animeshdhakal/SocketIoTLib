#ifndef SocketIoTClient_h
#define SocketIoTClient_h

#if !defined(FIRMWARE_VERSION) && !defined(BLUEPRINT_ID)
#error "FIRMWARE_VERSION and BLUEPRINT_ID must be defined"
#endif

#include "SocketIoTData.h"
#include "SocketIoTDebug.h"
#include "SocketIoTSpec.h"
#include "SocketIoTConfig.h"
#include "SocketIoTHandler.h"
#include "SocketIoTIdentifyDevice.h"

template <typename Connector>
class SocketIoTClient
{
protected:
    Connector &conn;
    const char *auth;
    SocketIoTState state;
    time_millis_t last_ping;
    time_millis_t last_recv;
    time_millis_t last_send;

public:
    SocketIoTClient(Connector &conn) : conn(conn), last_ping(0), last_send(0), last_recv(0), state(CONNECTING) {}

    void init(const char *auth, const char *host, uint16_t port)
    {
        this->auth = auth;
        this->conn.begin(host, port);
        LOG4(SF("Connecting to "), host, ":", port);
    }

    void init(const char *auth)
    {
        this->auth = auth;
        this->conn.begin(DEFAULT_HOST, DEFAULT_PORT);
        LOG4(SF("Connecting to "), DEFAULT_HOST, ":", DEFAULT_PORT);
    }

    void processWrite(SocketIoTData& data)
    {
        const int pin = data.toInt();
        
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

    void processSys(SocketIoTData& data){
        uint32_t cmd;
        memcpy(&cmd, data.toStr(), sizeof(cmd));

        if(++data >= data.end()) return;

        switch(cmd){
            case OTA_CMD:
                socketIoTOTA(data.toStr());  
                break;              
        }

    }

    template <typename T>
    void write(uint8_t pin, T value)
    {
        char buff[MAX_WRITE_BYTES];
        SocketIoTData data(buff, 0, sizeof(buff));
        data.put(pin);
        data.put(value);
        sendMsg(WRITE, data.toString(), data.length());
    }

    void processData()
    {
        SocketIoTHeader hdr;
        if (conn.read((uint8_t *)&hdr, sizeof(hdr)) == sizeof(SocketIoTHeader))
        {
            last_recv = MILLIS();
            hdr.msg_type = ntohs(hdr.msg_type);
            hdr.msg_len = ntohs(hdr.msg_len);

            if(hdr.msg_len > MAX_READ_BYTES){
                LOG1("Msg Bigger than Max Read Bytes");
                return;            
            }

            char buff[hdr.msg_len + 1];
            conn.read((uint8_t *)buff, hdr.msg_len);
            buff[hdr.msg_len] = 0;

            SocketIoTData data(buff, hdr.msg_len);

            switch (hdr.msg_type)
            {
            case AUTH:

                if (buff[0] - '0' == 1)
                {
                    LOG1(SF("Authenticated"));
                    LOG3("Ping ", last_recv - last_send, " MS");
                    state = CONNECTED;
                    sendInfo();
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
            case SYS:
                processSys(data);
                break;
            }
        }
    }

    void sendInfo()
    {
        static const char info[] SPROGMEM = "info\0" 
            InfoParam("hbeat", NumToString(HEARTBEAT)) 
            InfoParam("build", __DATE__ " " __TIME__) 
            InfoParam("fv", FIRMWARE_VERSION) 
            InfoParam("bid", BLUEPRINT_ID) 
            InfoParam("board", HARDWARE_NAME) 
            InfoParam("bid", BLUEPRINT_ID) 
            InfoParam("fv", FIRMWARE_VERSION) 
            InfoParam("lv", SOCKETIOT_VERSION)        
        "\0";

        size_t actualsize = sizeof(info) - 5 - 2;
        sendMsg(INFO, info + 5, actualsize);
    }

    void authenticate()
    {
        sendMsg(AUTH, auth, strlen(auth));
    }

    void sendMsg(uint16_t msg_type, const char *msg = NULL, uint16_t length = 0)
    {
        SocketIoTHeader hdr = {length, msg_type};
        hdr.msg_len = htons(hdr.msg_len);
        hdr.msg_type = htons(hdr.msg_type);
        uint8_t fullbuff[length + sizeof(SocketIoTHeader)];
        memcpy(fullbuff, &hdr, sizeof(SocketIoTHeader));
        memcpy(fullbuff + sizeof(SocketIoTHeader), msg, length);
        conn.write(fullbuff, length + sizeof(SocketIoTHeader));
        last_send = MILLIS();
    }

    bool connected()
    {
        return state == CONNECTED;
    }

    void disconnect(){
        state = DISCONNECTED;
        this->conn.disconnect();
    }

    bool authFailed()
    {
        return state == AUTH_FAILED;
    }

    void syncWithServer()
    {
        sendMsg(SYNC);
    }

    void loop()
    {
        YIELD();

        if(state == DISCONNECTED){
            return;
        }

        if (conn.connected())
        {
            while (conn.available() > 0)
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
                conn.disconnect();
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
            if (!conn.connected() && conn.connect())
            {
                LOG1(SF("Connected to Server"));
                authenticate();
            }
        }
    }
};

#endif
