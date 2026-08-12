#ifndef QURAN_H
#define QURAN_H

#include "../section8_global-header/global.h"

typedef struct {
    int surah;
    int ayah;
    const char* text;
} Ayah;

extern const Ayah quran[6236];

#endif // QURAN_H
