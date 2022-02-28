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

#endif
#endif