#include "../section8_global-header/global.h"
// 1. Calculate the length of a string
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// 2. Copy a string from source to destination
char* strcpy(char* dest, const char* src) {
    char* target = dest;
    while ((*dest++ = *src++)) {
        // Loops until it copies the '\0' null terminator
    }
    return target;
}

// 3. Compare two strings up to 'n' characters
int strncmp(const char* s1, const char* s2, size_t n) {
    if (n == 0) {
        return 0;
    }
    while (n-- > 0) {
        if (*s1 != *s2) {
            return (unsigned char)*s1 - (unsigned char)*s2;
        }
        if (*s1 == '\0') {
            break;
        }
        s1++;
        s2++;
    }
    return 0;
}
