#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LittleFS.h>

#define FIRMWARE_VERSION "1.0"
#define BLUEPRINT_ID "Animesh"
#define DEBUG

#include "SocketIoTEsp8266SSL.h"

enum DState {
    CONNECT_WIFI,
    CONNECT_SERVER,
    CONFIG_PORTAL,
    STATE_RUNNING
};

namespace DeviceState{
    DState state;
    DState get(){
        return state;
    }
    void set(DState stat){
        state = stat;
    }
};


struct Store {
    char ssid[32];
    char password[62];
    char host[30];
    char token[34];
    uint16_t port;
};

Store store;

void init_store(){
    if(!LittleFS.begin()){
        LOG1("LittleFS Failed");
        return;
    }

    File file = LittleFS.open("config", "r");

    if(file){
        file.read((uint8_t*)&store, sizeof(store));
        LOG1("Store Found");
        DeviceState::set(CONNECT_WIFI);
    }else{
        LOG1("Store Not Found");
        DeviceState::set(CONFIG_PORTAL);
    }

    file.close();
}

void save_store(){
    File file = LittleFS.open("config", "w");
    file.write((uint8_t*)&store, sizeof(store));
    file.close();
}

void reset_store(){
    LittleFS.remove("config");
    LOG1("Store Cleared ! Opening Portal");
    DeviceState::set(CONFIG_PORTAL);
}


volatile uint32_t press_time = 0;

ICACHE_RAM_ATTR
void btn_press(){
    if(!digitalRead(0)){
        press_time = millis();
    }else{
        if(millis() - press_time >= 6000 ){
            SocketIoT.disconnect();
            reset_store();
        }
        press_time = 0;
    }
}


void init_btn(){
    pinMode(0, INPUT_PULLUP);
    attachInterrupt(0, btn_press, CHANGE);
}

template <typename T, int size>
void StrCopy(String& str, T(&arr)[size]){
    str.toCharArray(arr, size);
}


static const char* ROOTPAGE PROGMEM = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Config</title>
</head>
<body>
    <form action="/config" method="get">
        <div>
            <label for="ssid">SSID:</label>
            <input type="text" name="ssid" id="ssid">
        </div>
        <div>
            <label for="password">Password:</label>
            <input type="text" name="password" id="password">
        </div>
        <div>
            <label for="host">Host:</label>
            <input type="text" name="host" id="host">
        </div>
        <div>
            <label for="port">Port:</label>
            <input type="number" name="port" id="port">
        </div>
        <div>
            <label for="token">Token:</label>
            <input type="text" name="token" id="token">
        </div>
        <div>
            <input type="submit" value="submit">
        </div>
    </form>
</body>
</html>
)html";


void connect_wifi(){
    LOG2("Connecting to ", store.ssid);
    WiFi.mode(WIFI_STA);

    WiFi.begin(store.ssid, store.password);

    uint32_t timeout = millis() + 10000;

    while(timeout > millis() && WiFi.status() != WL_CONNECTED){
        delay(10);
    }

    if(WiFi.status() == WL_CONNECTED){
        LOG1("WiFi Connected");
        DeviceState::set(CONNECT_SERVER);
        return;
    }

    LOG1("Timout! Opening Config Portal");

    DeviceState::set(CONFIG_PORTAL);
}


void getAPName(char* buff, size_t len){
    snprintf(buff, len, "SocketIoT-%s", String(ESP.getChipId(), HEX).c_str());
}

void open_portal(){
    ESP8266WebServer server(80);
    ESP8266HTTPUpdateServer updateServer;
    
    WiFi.mode(WIFI_OFF);
    delay(100);

    WiFi.mode(WIFI_STA);
    char buff[60];
    getAPName(buff, sizeof(buff));
    WiFi.softAP(buff);
    delay(300);

    LOG2("Opened AP: ", buff);
    LOG2("AP IP: ", WiFi.softAPIP().toString());
    
    server.on("/", [&](){
        server.send(200, "text/html", ROOTPAGE);
    });

    server.on("/wifiscan", [&](){
        int n = WiFi.scanNetworks(false, true);
        if(n > 0){
            int indices[n];
            for(int i = 0; i < n; i++){
                indices[i] = i;
            }

            for(int i = 0; i < n; i++){
                for(int j = i + 1; j < n; j++){
                    if(WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])){
                        std::swap(indices[i], indices[j]);
                    }
                }
            }

            server.setContentLength(CONTENT_LENGTH_UNKNOWN);
            server.send(200, "application/json", "[");

            char buff[256];

            for(int i = 0; i < n; i++){
                int id = indices[i];

                const char* enc;
                switch (WiFi.encryptionType(id)) {
                    case ENC_TYPE_WEP:  enc = "WEP"; break;
                    case ENC_TYPE_TKIP: enc = "WPA/PSK"; break;
                    case ENC_TYPE_CCMP: enc = "WPA2/PSK"; break;
                    case ENC_TYPE_AUTO: enc = "WPA/WPA2/PSK"; break;
                    case ENC_TYPE_NONE: enc = "OPEN"; break;
                    default:            enc = "unknown"; break;
                }

                snprintf(buff, sizeof(buff),
                    R"json({"ssid":"%s", "bssid":"%s", "rssi":%i, "enc":"%s", "channel":%i, "hidden":%d})json",
                    WiFi.SSID(id),
                    WiFi.BSSIDstr(id).c_str(),
                    WiFi.RSSI(id),
                    enc,
                    WiFi.channel(id),
                    WiFi.isHidden(id)
                );
                server.sendContent(buff);
                if(i != n - 1){
                    server.sendContent(",");
                }
            }
            server.sendContent("]");
        }else{
            server.send(200, "application/json", "[]");
        }

    });

    server.on("/config", [&](){
        String ssid = server.arg("ssid");
        String password = server.arg("password");
        String host = server.arg("host");
        String port = server.arg("port");
        String token = server.arg("token");
 

        if(ssid.length() && password.length() && host.length() && token.length() && port.length()){
            StrCopy(ssid, store.ssid);
            StrCopy(password, store.password);
            StrCopy(host, store.host);
            StrCopy(token, store.token);

            store.port = port.toInt();
                        
            save_store();

            server.send(200, "application/json", R"json({error:false, message:"Saved Config and Trying to Connect"})json");
        
            delay(100);

            DeviceState::set(CONNECT_WIFI);

        }else{
            server.send(200, "application/json", R"json({error: true, message:"Incomplete Fields"})json");
        }
    });

    updateServer.setup(&server, "/update");

    server.begin();

    while(DeviceState::state == CONFIG_PORTAL){
        delay(10);
        server.handleClient();
    }

    server.stop();    
}


void connect_server(){
    LOG1("Connecting to Server");

    SocketIoT.init(store.token, store.host, store.port);

    time_millis_t timeout = MILLIS() + 6000;

    while(timeout > millis() && !SocketIoT.connected() && !SocketIoT.authFailed()){
        delay(10);
        SocketIoT.loop();
    }

    if(millis() > timeout){
        LOG1("Server Connection Timeout");
        return;
    }

    if(SocketIoT.authFailed()){
        LOG1("Auth Failed! Opening Portal");
        DeviceState::set(CONFIG_PORTAL);
    }else{
        DeviceState::set(STATE_RUNNING);
    }
}


bool parseURL(String url, String& protocol, String& host, String& path, int& port){
    int index = url.indexOf(":");
    
    if(index < 0){
        return false;
    }
    
    protocol = url.substring(0, index);
    url.remove(0, index + 3);

    index = url.indexOf("/");
    host = url.substring(0, index);
    
    url.remove(0, index);
    index = url.indexOf(":");
    
    if(protocol == "https"){
        port = 443;
    }else{
        port = 80;
    }

    if(url.length()){
        path = url;
    }else{
        path = "/";
    }
    

   return true;
}


void restartMCU(){
    ESP.restart();
    delay(1000);
    ESP.reset();
    while (1)
    {
    };
}

void start_ota(const char* url){
    String protocol, host, path;
    int port;
    if(!parseURL(url, protocol, host, path, port)){
        LOG1("OTA Failed : Invalid URL");
        return;
    }

    WiFiClient::stopAll();
    WiFiUDP::stopAll();

    syncTime();

    WiFiClientSecure client;

    client.setTrustAnchors(&SocketIoTCert);

    if(!client.connect(host, port)){
        LOG1("OTA Client Connection Failed");
        return;
    }

    client.print(String("GET ") + path + " HTTP/1.0\r\n"
               + "Host: " + host + "\r\n"
               + "Connection: keep-alive\r\n"
               + "\r\n");

    uint32_t timeout = millis();
    while(client.connected() && !client.available()){
        if(millis() - timeout > 10000L){
            LOG1("Response Timeout");
        }
        delay(10);
    }


    int contentLength = 0;
    while(client.available()){
        String line = client.readStringUntil('\n');
        line.trim();
        line.toLowerCase();

        if(line.startsWith("content-length")){
            contentLength = line.substring(line.lastIndexOf(':') + 1).toInt();
        }else if(line.length() == 0){
            break;
        }
        delay(10);
    }

    LOG2("Content-Length: ", contentLength);

    if(!Update.begin(contentLength)){
        LOG1("Update Begin Failed");
        return;
    }

    int written = 0;
    int prevProgress = 0;
    uint8_t buff[256];

    while(client.connected() && written < contentLength){
        delay(10);
        timeout = millis();

        while(client.connected() && !client.available()){
            delay(1);
            if(millis() - timeout > 10000L){
                LOG1("Timeout");
                return;
            }
        }

        int rlen = client.read(buff, sizeof(buff));

        if(rlen <= 0) continue;

        Update.write(buff, rlen);
        written += rlen;

        int progress = (written*100)/contentLength;
        if(progress - prevProgress >= 10 || progress == 100){
            LOG3("Written ", progress, "%");
            prevProgress = progress;
        }
    }

    client.stop();

    if(written != contentLength){
        LOG1("OTA Writing Failed");
    }

    if(!Update.end()){
        LOG1("Update Not Ended");
    }

    if(!Update.isFinished()){
        LOG1("Update Not Finished");
    }

    LOG1("Update Finished ! Rebooting");

    restartMCU();
}

SocketIoTConnected(){
    SocketIoT.syncWithServer();
}

SocketIoTWrite(1){
    digitalWrite(LED_BUILTIN, data.toInt());
}

SocketIoTOTA(){
    LOG2("Starting OTA, URL: ", url);
    start_ota(url);
}

void printDeviceInfo(){
    LOG1(SF("\n---------------------INFO---------------------"))
    LOG1(String("Firmware: ") + FIRMWARE_VERSION " (build " __DATE__ " " __TIME__ ")");
    LOG1(String("Device: ") + HARDWARE_NAME + "@" + ESP.getCpuFreqMHz() + "MHz");
    LOG1(String("MAC: ") + WiFi.macAddress());
    LOG1(String("Flash: ") + ESP.getFlashChipRealSize()/1024 + "KB");
    String coreVersion = ESP.getCoreVersion();
    coreVersion.replace("-", ".");
    LOG1(String("Core: ")  + coreVersion);
    LOG1(String("SDK: ") + ESP.getSdkVersion());
    LOG1(String("Boot Version: ") + ESP.getBootVersion());
    LOG1(String("Boot Mode: ") + ESP.getBootMode());
    LOG1(String("Firmware Info: ") + ESP.getSketchSize() + "/" + ESP.getFreeSketchSpace() + ", MD5:" + ESP.getSketchMD5());
    LOG1(String("Free Memory: ") + ESP.getFreeHeap()/1024 + "KB");
    LOG1(SF("---------------------INFO---------------------"))
}

void setup()
{
    Serial.begin(115200);
    printDeviceInfo();
    init_btn();
    init_store();
    pinMode(LED_BUILTIN, OUTPUT); 
}


void loop()
{
   switch(DeviceState::state){
        case CONNECT_WIFI:
            connect_wifi();
            break;
        case CONFIG_PORTAL:
            open_portal();
            break;
        case CONNECT_SERVER:
            connect_server();
            break;
        case STATE_RUNNING:
            SocketIoT.loop();
            break;
   }
}