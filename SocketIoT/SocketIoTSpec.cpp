#include "SocketIoTSpec.h"

#if defined(linux)
#include <time.h>
#include <unistd.h>

time_millis_t start_time = MILLIS();
time_millis_t MILLIS(){
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (t.tv_sec * 1000 + t.tv_nsec / 1000000) - start_time;
}

void DELAY(time_millis_t t){
    usleep(t);
}

#else
#include <Arduino.h>
time_millis_t MILLIS()
{
    return millis();
}

void DELAY(time_millis_t t){
    delay(t);
}

#endif