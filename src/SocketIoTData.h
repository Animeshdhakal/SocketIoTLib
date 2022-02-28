#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

class SocketIOTData
{
public:
    char *buff;
    size_t buff_len;

    SocketIOTData(char *buff, size_t buff_len)
    {
        this->buff = buff;
        this->buff_len = buff_len;
    }

    bool is_valid()
    {
        return buff != NULL && buff < (buff + buff_len);
    }

    SocketIOTData &operator++()
    {
        if (is_valid())
        {
            buff += strlen(buff) + 1;
        }
        return *this;
    }
    SocketIOTData &operator--()
    {
        if (is_valid())
        {
            buff += strlen(buff) + 1;
        }
        return *this;
    }

    const char *toString()
    {
        return buff;
    }

    const char *toStr()
    {
        return buff;
    }

    int toInt()
    {
        if (is_valid())
        {
            return atoi(buff);
        }
        return 0;
    }

    float toFloat()
    {
        if (is_valid())
        {
            return atof(buff);
        }
        return 0;
    }

    const char *end()
    {
        return buff + buff_len;
    }

    size_t length()
    {
        return buff_len;
    }

    bool operator<(const char *data) const
    {
        return buff < data;
    }
    bool operator>=(const char *data) const
    {
        return buff >= data;
    }
};