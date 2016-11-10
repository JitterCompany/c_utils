#include "str.h"

void str_toupper(char *str)
{
    while(*str != '\0') {
        *str = toupper(*str);
        str++;
    }
}

