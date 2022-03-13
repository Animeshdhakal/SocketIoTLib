#include <iostream>
#include <string>

#define FIRMWARE_VERSION "1.0"
#define BLUEPRINT_ID "Animesh"
#define DEBUG

#include "SocketIoTLinuxSSLConnector.h"
#include "SocketIoTClient.h"
#include "StringUtil.h"

static SocketIoTLinuxConnnector connector;
static SocketIoTClient<SocketIoTLinuxConnnector> SocketIoT(connector);

SocketIoTWrite(1){
    LOG1(data.toInt());
}

void parseURL(char* url, char* host, int& port, char* path){
    int index = StringUtil.indexOf(url, ':');
    url[index + 1] = 0;
    if(strcmp(url, "https"))
    {
        port = 443;
    }else if(strcmp(url, "http")){
        port = 80;
    }
    url += index + 3;
    index = StringUtil.indexOf(url, '/');
    url[index] = 0;
    strcpy(host, url);
    url[index] = '/';
    url += index; 
    strcpy(path, url);
}

void start_ota(const char* u){
    char url[100], host[60];
    int port;

    strcpy(url, u);
    parseURL(url, host, port, url);

    SocketIoT.disconnect();

    connector.begin(host, port);

    if(!connector.connect()){
        std::cout << "Connection Failed";
    }

    std::string getReq = std::string("GET ") + url + " HTTP/1.1\r\nHost: " + host + "\r\n\r\n";

    connector.write((uint8_t*)getReq.c_str(), getReq.length());

    uint8_t buff[1 * 1024 * 1024];

    connector.read(buff, sizeof(buff));

    char* pos = (char*)buff;

    int contentLength = 0;
    while(true){
        int index = StringUtil.indexOf(pos, '\n');
        if(index <= 0 || index > sizeof(buff)) break;

        pos[index - 1] = 0;
        
        if(strlen(pos) <= 0) {
            pos += index + 1;
            break;
        }

        if(StringUtil.startsWith(pos, "content-length")){
            contentLength = atoi(pos + StringUtil.indexOf(pos, ':')+1);
        }

        pos += index + 1;
    }
    

    FILE* file = fopen("newfirmware", "wb");
    fwrite(pos, contentLength, 1, file);
    fclose(file); 
}

int main(){
    SocketIoT.init("", "", 443);

    for(;;){
        SocketIoT.loop();
    }
}