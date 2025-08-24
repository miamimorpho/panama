#ifndef TERM_H
#define TERM_H

#include <stdint.h>
#include "utf32.h"

struct TermUI{
    int32_t x;
    int32_t y;
};

enum termInputCodes{
    T_KEY_NONE = 0,
    T_KEY_ASCII_START = 32,
    T_KEY_ASCII_END = 126,
    T_KEY_ESCAPE = 256,
    T_KEY_UP,
    T_KEY_DOWN,
    T_KEY_LEFT,  
    T_KEY_RIGHT,
    T_KEY_F1,
};

void termInit(void);
void termRefresh(void);
void termCh(struct TermUI ui, utf32_t ch);
void termClear(void);
void termSize(uint16_t *, uint16_t *);
int64_t termIn(void);

#endif // TERM_H
