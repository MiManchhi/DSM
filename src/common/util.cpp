#include "util.h"
#include <string.h>

void llton(long long ll, char *n)
{
    for (size_t i = 0; i < sizeof(long long); i++)
    {
        n[i] = ll >> (sizeof(ll) - i -1)*8;
    }
}

long long ntoll(char const *n)
{
    long long ll = 0;
    for (size_t i = 0; i < sizeof(long long); i++)
    {
        ll |= (long long)(unsigned char)n[i] << (sizeof(long long) -i - 1)*8;
    }
    return ll;
}

void lton(long l, char *n)
{
    for (size_t i = 0; i < sizeof(long); i++)
    {
        n[i] = l >> (sizeof(l) - i -1)*8;
    }
}

long ntol(char const *n)
{
    long l = 0;
    for (size_t i = 0; i < sizeof(long long); i++)
    {
        l |= (long)(unsigned char)n[i] << (sizeof(long) -i - 1)*8;
    }
    return l;
}

void ston(short s, char *n)
{
    for (size_t i = 0; i < sizeof(short); i++)
    {
        n[i] = s >> (sizeof(s) - i -1)*8;
    }
}

short ntos(char const *n)
{
    short s = 0;
    for (size_t i = 0; i < sizeof(short); i++)
    {
        s |= (short)(unsigned char)n[i] << (sizeof(short) -i - 1)*8;
    }
    return s;
}

int valid(char const *str)
{
    if(!str)
        return ERROR;
    size_t len = strlen(str);
    if(!len)
        return ERROR;
    for (size_t i = 0; i < len; i++)
    {
        if(!(('a' <= str[i] && str[i] <= 'z') || 
            ('A' <= str[i] && str[i] <= 'Z') ||
            ('0' <= str[i] && str[i] <= '9')))
            return ERROR;
    }
    return OK;
}

int splitstring(char const *str, std::vector<std::string> &substrs)
{
    if(!str)
        return ERROR;
    size_t len = strlen(str);
    if(!len)
        return ERROR;
    char* buf = new char[len+1];
    strcpy(buf,str);
    char const* sep = ";";
    for(char* substr = strtok(buf,sep);substr;substr = strtok(NULL,sep))
        substrs.push_back(substr);
    delete[] buf;
    return OK;
}