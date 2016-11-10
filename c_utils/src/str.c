#include "str.h"
#include <ctype.h>

void str_toupper(char *str)
{
    while(*str != '\0') {
        *str = toupper(*str);
        str++;
    }
}

