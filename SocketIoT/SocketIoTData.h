#ifndef SocketIoTData_h
#define SocketIoTData_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

class SocketIoTData
{
public:
    char *buff;
    size_t buff_len;
    size_t buff_size;

    SocketIoTData(char *buff, size_t buff_len, size_t buff_size = 0)
    {
        this->buff = buff;
        this->buff_len = buff_len;
        this->buff_size = buff_size;
    }

    bool isValid()
    {
        return buff != NULL && buff < (buff + buff_len);
    }

    SocketIoTData &operator++()
    {
        if (isValid())
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
        if (isValid())
        {
            return atoi(buff);
        }
        return 0;
    }

    float toFloat()
    {
        if (isValid())
        {
            return atof(buff);
        }
        return 0;
    }

    void put(const char *str, size_t slen)
    {
        memcpy(buff + buff_len, str, slen);
        buff_len += slen;
        buff[buff_len] = 0;
    }

    void put(const char *str)
    {
        put(str, strlen(str) + 1);
    }

    void put(int val)
    {
        buff_len += snprintf(buff + buff_len, buff_size - buff_len, "%d", val) + 1;
        buff[buff_len] = 0;
    }

    void put(float val)
    {
        buff_len += snprintf(buff + buff_len, buff_size - buff_len, "%f", val) + 1;
        buff[buff_len] = 0;
    }

    void put(double val)
    {
        buff_len += snprintf(buff + buff_len, buff_size - buff_len, "%lf", val) + 1;
        buff[buff_len] = 0;
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

#endif