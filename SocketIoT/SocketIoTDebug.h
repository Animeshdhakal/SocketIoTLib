#ifndef SocketIoTDebug_h
#define SocketIoTDebug_h

#include "SocketIoTSpec.h"

#if defined(linux)
#include <iostream>
void LOG_TIME()
{
    std::cout << SF("[");
    std::cout << MILLIS();
    std::cout << SF("] ");
}

template <typename T>
void DEBUG_MULTI(T msg)
{
    std::cout << msg << "\n";
}

template <typename T, typename... Args>
void DEBUG_MULTI(T msg, Args... args)
{
    std::cout << msg;
    DEBUG_MULTI(args...);
}

template <typename... Args>
void DEBUG(Args... args)
{
    LOG_TIME();
    DEBUG_MULTI(args...);
}
#else

void LOG_TIME()
{
    Serial.print(SF("["));
    Serial.print(MILLIS());
    Serial.print(SF("] "));
}

#ifdef DEBUG

#define LOG1(msg) \
    LOG_TIME();   \
    Serial.println(msg);

#define LOG2(msg, msg2) \
    LOG_TIME();         \
    Serial.print(msg);  \
    Serial.println(msg2);

#define LOG3(msg, msg2, msg3) \
    LOG_TIME();               \
    Serial.print(msg);        \
    Serial.print(msg2);       \
    Serial.println(msg3);

#define LOG4(msg, msg2, msg3, msg4) \
    LOG_TIME();                     \
    Serial.print(msg);              \
    Serial.print(msg2);             \
    Serial.print(msg3);             \
    Serial.println(msg4);

#else

#define LOG1

#define LOG2

#define LOG3

#define LOG4

#endif

#endif

#endif