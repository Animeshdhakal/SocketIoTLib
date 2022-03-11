#ifndef SocketIoTArduinoConnector_h
#define SocketIoTArduinoConnector_h

#include "SocketIoTSpec.h"

template <typename Client>
class SocketIoTArduinoConnector
{
    protected:
    const char* host;
    uint16_t port;
    Client& client;

    public:
    SocketIoTArduinoConnector(Client& client) : client(client), host(NULL), port(0){}

    void begin(const char* host, uint16_t port){
        this->host = host;
        this->port = port;
    }

    bool connect(){
        return client.connect(host, port) == 1;
    }

    void disconnect(){
        client.stop();
    }

    size_t write(const void* buff, size_t len){
        return client.write((uint8_t*)buff, len);
    }

    size_t read(void* buff, size_t len){
        return client.read((uint8_t*)buff, len);
    }

    bool connected(){
        return client.connected();
    }

    int available(){
        return client.available();
    }
    
};


#endif