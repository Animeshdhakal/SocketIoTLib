#ifndef SocketIoTDebug_h
#define SocketIoTDebug_h

#include "SocketIoTSpec.h"


#ifdef DEBUG

#if defined(linux)

#include <iostream>

void LOG_TIME()
{
    std::cout << SF("[") << MILLIS() << SF("] ");
}
#define LOG1(msg) \
    LOG_TIME();   \
    std::cout << msg << "\n";

#define LOG2(msg, msg2) \
    LOG_TIME();         \
    std::cout << msg << msg2 << "\n";

#define LOG3(msg, msg2, msg3) \
    LOG_TIME();               \
    std::cout << msg << msg2 << msg3 << "\n";

#define LOG4(msg, msg2, msg3, msg4) \
    LOG_TIME();                     \
    std::cout << msg << msg2 << msg3 << msg4 << "\n";

#else

void LOG_TIME()
{
    Serial.print(SF("["));
    Serial.print(MILLIS());
    Serial.print(SF("] "));
}

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

#endif

#else
#define LOG1
#define LOG2
#define LOG3
#define LOG4
#endif

#endif