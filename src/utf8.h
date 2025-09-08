#ifndef UTF8_HELPER
#define UTF8_HELPER

#include <stdint.h>

typedef struct {
	char bytes[4];
} utf8_ch;
typedef const char *utf8_str;

static const utf8_ch UTF8_NULL = {0};

int utf8Equal(utf8_ch, utf8_ch);
void utf8Put(utf8_ch);
utf8_ch utf8Get(int fd);
utf8_ch utf8Char(utf8_str);
unsigned int utf8ChSize(utf8_str s);
utf8_ch utf8Code(uint32_t);
int utf8Next(utf8_str *str);

#endif // UTF8_HELPER
