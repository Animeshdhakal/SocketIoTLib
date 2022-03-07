#ifndef SocketIoTSpec_h
#define SocketIoTSpec_h

#include <stdint.h>

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
    INFO,
    SYS
};

enum SocketIoTState
{
    CONNECTING,
    CONNECTED,
    AUTH_FAILED
};

#if defined(ESP32) || defined(ESP8266)
#include <lwip/ip_addr.h>
#elif !defined(htons) && (defined(ARDUINO) || defined(PARTICLE) || defined(__MBED__))
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define htons(x) (((x) << 8) | (((x) >> 8) & 0xFF))
#define htonl(x) (((x) << 24 & 0xFF000000UL) | \
                  ((x) << 8 & 0x00FF0000UL) |  \
                  ((x) >> 8 & 0x0000FF00UL) |  \
                  ((x) >> 24 & 0x000000FFUL))
#define ntohs(x) htons(x)
#define ntohl(x) htonl(x)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define htons(x) (x)
#define htonl(x) (x)
#define ntohs(x) (x)
#define ntohl(x) (x)
#else
#error byte order problem
#endif
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define STR32(a,b,c,d) ((uint32_t(a) << 0) | (uint32_t(b) << 8) | (uint32_t(c) << 16) | (uint32_t(d) << 24))
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define STR32(a,b,c,d) ((uint32_t(d) << 0) | (uint32_t(c) << 8) | (uint32_t(b) << 16) | (uint32_t(a) << 24))
#else
    #error byte order problem
#endif

typedef uint32_t time_millis_t;

time_millis_t MILLIS()
{
    return millis();
}

#define DELAY delay

#define YIELD() DELAY(0);

#if defined(__AVR__)

#define SF(s) F(s)

#else

#define SF(s) s

#endif

#define Num2String(x) #x

#define NumToString(x) Num2String(x)

#define InfoParam(x, y) x "\0" y "\0"

#define SPROGMEM PROGMEM

#define OTA_CMD STR32('o', 't', 'a', 0)

#endif