#ifndef StringUtil_h
#define StringUtil_h

#include <string.h>
#include <ctype.h>

class _StringUtil{
    public:
    bool startsWith(const char* str, const char* substr){
        return strncmp(str, substr, strlen(substr)) == 0;
    }

    int indexOf(const char* str, char c){
        return strchr(str, c) - str;
    }

    void toLower(char* str){
        for(int i = 0; i < strlen(str); i++){
            str[i] = tolower(str[i]);
        }
    }
};

static _StringUtil StringUtil;

#endif