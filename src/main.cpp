#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "SocketIoTData.h"

struct SocketIoTHeader
{
    uint16_t msg_len;
    uint16_t msg_type;
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

typedef uint32_t time_millis_t;

time_millis_t MILLIS()
{
    return millis();
}

#define SOCKETIOT_PROGMEM PROGMEM
#define YIELD yield
#define HEARTBEAT 30
#define Num2String(x) #x
#define NumToString(x) Num2String(x)
#define InfoParam(x, y) x "\0" y "\0"
#if defined(__AVR__)
#define S_F(x) F(x)
#else
#define S_F(x) x
#endif

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

void LOG_TIME()
{
    Serial.print(S_F("["));
    Serial.print(MILLIS());
    Serial.print(S_F("] "));
}

template <typename T>
void DEBUG_MULTI(T msg)
{
    Serial.println(msg);
}

template <typename T, typename... Args>
void DEBUG_MULTI(T msg, Args... args)
{
    Serial.print(msg);
    DEBUG_MULTI(args...);
}

template <typename... Args>
void DEBUG(Args... args)
{
    LOG_TIME();
    DEBUG_MULTI(args...);
}

enum SocketIoTState
{
    CONNECTING,
    CONNECTED,
    AUTH_FAILED,
};

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
    }

    void processWrite(SocketIOTData data)
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

            SocketIOTData data(buff, hdr.msg_len);

            switch (hdr.msg_type)
            {
            case AUTH:

                if (buff[0] - '0' == 1)
                {
                    DEBUG("Authenticated");
                    sendInfo();
                    state = CONNECTED;
                }
                else
                {
                    DEBUG("Authentication failed");
                    state = AUTH_FAILED;
                }
                break;
            case PING:
                DEBUG("PING");
                break;
            case WRITE:
                processWrite(data);
                break;
            }
        }
    }

    void sendInfo()
    {
        static const char info[] SOCKETIOT_PROGMEM = "info\0" InfoParam("hbeat", NumToString(HEARTBEAT)) InfoParam("build", __DATE__ " " __TIME__) "\0";
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
                DEBUG("Disconnected from Server");
                state = CONNECTING;
            }
            else if (now - last_ping > HEARTBEAT * 1000 && (now - last_recv > HEARTBEAT * 1000 || now - last_send > HEARTBEAT * 1000))
            {
                DEBUG("Sending PING");
                sendMsg(PING);
                last_ping = now;
            }
        }
        else if (state == CONNECTING)
        {
            if (!client.connected() && client.connect(host, port))
            {
                DEBUG("Connected to Server");
                authenticate();
            }
        }
    }
};

static const char *CERT SOCKETIOT_PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFHjCCBAagAwIBAgISBP90FGsov4jmZSrPqVsKxyUaMA0GCSqGSIb3DQEBCwUA
MDIxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQD
EwJSMzAeFw0yMjAyMTgwOTQ0MTRaFw0yMjA1MTkwOTQ0MTNaMBcxFTATBgNVBAMT
DHVuaHB1LmNvbS5ucDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAIU2
qG9z4Pu6duiOwgybCVRGZ4+y2mLjiKKZIm2kdLr8a5L3rvVx8WAVNJZhtOKQCVc9
IujLLDPoHuohVXFGNklbxIf39YX2mgxfVv9owVivjmY0OlVUr7upVDGc2g+mivk3
/1ucwKhg5Ez+EjP/Qf5IcpTa8YOcZhBHV/42yJEvT0YP2hhasDn9yF2qGrhfeX7q
5FRxSOZAX8WYZ4YNzDTDbLZGTJ+O7ALKQNEnIfpRDhQyyhBxp5+9ZbI5yKv3Qiau
l3zbpSYzoprSDmBXyrHcjU5qF4PkWM2R+ZAOb1xS4vDNoEeGHs/ZOwfxT7J+mxWi
AwBdPqtn+p35M3ElBrsCAwEAAaOCAkcwggJDMA4GA1UdDwEB/wQEAwIFoDAdBgNV
HSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDAYDVR0TAQH/BAIwADAdBgNVHQ4E
FgQU54b0VxrRXRzqyBhkRjnLz9ZfQwgwHwYDVR0jBBgwFoAUFC6zF7dYVsuuUAlA
5h+vnYsUwsYwVQYIKwYBBQUHAQEESTBHMCEGCCsGAQUFBzABhhVodHRwOi8vcjMu
by5sZW5jci5vcmcwIgYIKwYBBQUHMAKGFmh0dHA6Ly9yMy5pLmxlbmNyLm9yZy8w
FwYDVR0RBBAwDoIMdW5ocHUuY29tLm5wMEwGA1UdIARFMEMwCAYGZ4EMAQIBMDcG
CysGAQQBgt8TAQEBMCgwJgYIKwYBBQUHAgEWGmh0dHA6Ly9jcHMubGV0c2VuY3J5
cHQub3JnMIIBBAYKKwYBBAHWeQIEAgSB9QSB8gDwAHYA36Veq2iCTx9sre64X04+
WurNohKkal6OOxLAIERcKnMAAAF/DG6t6AAABAMARzBFAiBbjnIr79uOQLcOv5Go
D1h4a1l2/MMUMj46d2UFQd4AtAIhAMuMMzbXYZoEBUa4oCKDbdWqseeiCJ0b5RmQ
VDkTJ+1tAHYAKXm+8J45OSHwVnOfY6V35b5XfZxgCvj5TV0mXCVdx4QAAAF/DG6v
zAAABAMARzBFAiBev2SS9+9WVpWplQIbafNAioSzsqO9xfBEuO4CnGNH8QIhAL62
gCWvVeM1UR8d1utyWUhxd2ts178qJLNjtyr7d3ObMA0GCSqGSIb3DQEBCwUAA4IB
AQCnvjUUMwoZmX8ZHhIW4q/bnijTV2zBq0nUqpEPcww8KRvpjLTKqWs+/je32Ryw
TYB8ANwxpfY2slxNOB/JeZ90Eg/9VZMnrCgOMh7rpLZVef8XX/8tlllwrXb3zSan
IKhqETksLsLzsedpnBdrNc1Lc1GSoew03MLvOgR6enHwWeWzt6f+zgOGz6Cq54Pz
23NzY4rXCSREdsJ22dTHEiVohR9gCeP3F3VjojJoUGLxObyfrZuGLjUF79uvdKtQ
e1oe4EXZqH7Xnt266qIRyL6x4dy2u/1uv8Kfyx4JQHNxvQH34m3hEfzFq6ic23Rt
hujmpPI7rjgMFYcGU9OfRnv4
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIFFjCCAv6gAwIBAgIRAJErCErPDBinU/bWLiWnX1owDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjAwOTA0MDAwMDAw
WhcNMjUwOTE1MTYwMDAwWjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg
RW5jcnlwdDELMAkGA1UEAxMCUjMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK
AoIBAQC7AhUozPaglNMPEuyNVZLD+ILxmaZ6QoinXSaqtSu5xUyxr45r+XXIo9cP
R5QUVTVXjJ6oojkZ9YI8QqlObvU7wy7bjcCwXPNZOOftz2nwWgsbvsCUJCWH+jdx
sxPnHKzhm+/b5DtFUkWWqcFTzjTIUu61ru2P3mBw4qVUq7ZtDpelQDRrK9O8Zutm
NHz6a4uPVymZ+DAXXbpyb/uBxa3Shlg9F8fnCbvxK/eG3MHacV3URuPMrSXBiLxg
Z3Vms/EY96Jc5lP/Ooi2R6X/ExjqmAl3P51T+c8B5fWmcBcUr2Ok/5mzk53cU6cG
/kiFHaFpriV1uxPMUgP17VGhi9sVAgMBAAGjggEIMIIBBDAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMBMBIGA1UdEwEB/wQIMAYB
Af8CAQAwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYfr52LFMLGMB8GA1UdIwQYMBaA
FHm0WeZ7tuXkAXOACIjIGlj26ZtuMDIGCCsGAQUFBwEBBCYwJDAiBggrBgEFBQcw
AoYWaHR0cDovL3gxLmkubGVuY3Iub3JnLzAnBgNVHR8EIDAeMBygGqAYhhZodHRw
Oi8veDEuYy5sZW5jci5vcmcvMCIGA1UdIAQbMBkwCAYGZ4EMAQIBMA0GCysGAQQB
gt8TAQEBMA0GCSqGSIb3DQEBCwUAA4ICAQCFyk5HPqP3hUSFvNVneLKYY611TR6W
PTNlclQtgaDqw+34IL9fzLdwALduO/ZelN7kIJ+m74uyA+eitRY8kc607TkC53wl
ikfmZW4/RvTZ8M6UK+5UzhK8jCdLuMGYL6KvzXGRSgi3yLgjewQtCPkIVz6D2QQz
CkcheAmCJ8MqyJu5zlzyZMjAvnnAT45tRAxekrsu94sQ4egdRCnbWSDtY7kh+BIm
lJNXoB1lBMEKIq4QDUOXoRgffuDghje1WrG9ML+Hbisq/yFOGwXD9RiX8F6sw6W4
avAuvDszue5L3sz85K+EC4Y/wFVDNvZo4TYXao6Z0f+lQKc0t8DQYzk1OXVu8rp2
yJMC6alLbBfODALZvYH7n7do1AZls4I9d1P4jnkDrQoxB3UqQ9hVl3LEKQ73xF1O
yK5GhDDX8oVfGKF5u+decIsH4YaTw7mP3GFxJSqv3+0lUFJoi5Lc5da149p90Ids
hCExroL1+7mryIkXPeFM5TgO9r0rvZaBFOvV2z0gp35Z0+L4WPlbuEjN/lxPFin+
HlUjr8gRsI3qfJOQFy/9rKIJR0Y/8Omwt/8oTWgy1mdeHmmjk7j1nYsvC9JSQ6Zv
MldlTTKB3zhThV1+XWYp6rjd5JW1zbVWEkLNxE7GJThEUG3szgBVGP7pSWTUTsqX
nLRbwHOoq7hHwg==
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIFYDCCBEigAwIBAgIQQAF3ITfU6UK47naqPGQKtzANBgkqhkiG9w0BAQsFADA/
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT
DkRTVCBSb290IENBIFgzMB4XDTIxMDEyMDE5MTQwM1oXDTI0MDkzMDE4MTQwM1ow
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwggIiMA0GCSqGSIb3DQEB
AQUAA4ICDwAwggIKAoICAQCt6CRz9BQ385ueK1coHIe+3LffOJCMbjzmV6B493XC
ov71am72AE8o295ohmxEk7axY/0UEmu/H9LqMZshftEzPLpI9d1537O4/xLxIZpL
wYqGcWlKZmZsj348cL+tKSIG8+TA5oCu4kuPt5l+lAOf00eXfJlII1PoOK5PCm+D
LtFJV4yAdLbaL9A4jXsDcCEbdfIwPPqPrt3aY6vrFk/CjhFLfs8L6P+1dy70sntK
4EwSJQxwjQMpoOFTJOwT2e4ZvxCzSow/iaNhUd6shweU9GNx7C7ib1uYgeGJXDR5
bHbvO5BieebbpJovJsXQEOEO3tkQjhb7t/eo98flAgeYjzYIlefiN5YNNnWe+w5y
sR2bvAP5SQXYgd0FtCrWQemsAXaVCg/Y39W9Eh81LygXbNKYwagJZHduRze6zqxZ
Xmidf3LWicUGQSk+WT7dJvUkyRGnWqNMQB9GoZm1pzpRboY7nn1ypxIFeFntPlF4
FQsDj43QLwWyPntKHEtzBRL8xurgUBN8Q5N0s8p0544fAQjQMNRbcTa0B7rBMDBc
SLeCO5imfWCKoqMpgsy6vYMEG6KDA0Gh1gXxG8K28Kh8hjtGqEgqiNx2mna/H2ql
PRmP6zjzZN7IKw0KKP/32+IVQtQi0Cdd4Xn+GOdwiK1O5tmLOsbdJ1Fu/7xk9TND
TwIDAQABo4IBRjCCAUIwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYw
SwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5pZGVudHJ1
c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTEp7Gkeyxx
+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEEAYLfEwEB
ATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2VuY3J5cHQu
b3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0LmNvbS9E
U1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFHm0WeZ7tuXkAXOACIjIGlj26Ztu
MA0GCSqGSIb3DQEBCwUAA4IBAQAKcwBslm7/DlLQrt2M51oGrS+o44+/yQoDFVDC
5WxCu2+b9LRPwkSICHXM6webFGJueN7sJ7o5XPWioW5WlHAQU7G75K/QosMrAdSW
9MUgNTP52GE24HGNtLi1qoJFlcDyqSMo59ahy2cI2qBDLKobkx/J3vWraV0T9VuG
WCLKTVXkcGdtwlfFRjlBz4pYg1htmf5X6DYO8A4jqv2Il9DjXA6USbW1FzXSLr9O
he8Y4IWS6wY7bCkjCWDcRQJMEhg76fsO3txE+FiYruq9RUWhiF1myv4Q6W+CyBFC
Dfvp7OOGAN6dEOM4+qR9sdjoSYKEBpsr6GtPAQw4dy753ec5
-----END CERTIFICATE-----
)EOF";

static X509List SocketIoTCert(CERT);

static WiFiClientSecure client;
static SocketIoTClient<WiFiClientSecure> socketIoT(client);

SocketIoTWrite(1)
{
    Serial.println("SocketIOTWrite");
    Serial.println(data.toInt());
    digitalWrite(LED_BUILTIN, data.toInt());
}

void syncTime()
{
    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    Serial.print("Waiting for NTP time sync: ");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2)
    {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }
    Serial.println("");
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.print(asctime(&timeinfo));
}

void setup()
{

    Serial.begin(115200);
    WiFi.begin("", "");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    syncTime();

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("Connected to WiFi");

    client.setTrustAnchors(&SocketIoTCert);

    socketIoT.init("", "", 443);
}

void loop()
{
    socketIoT.loop();
}
