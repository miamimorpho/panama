#ifndef UTF8_HELPER
#define UTF8_HELPER

#include <stdint.h>

#define UTF8_BYTES_SIZE 4

typedef struct {
	char bytes[UTF8_BYTES_SIZE];
	int size;
} utf8_ch;
typedef const char utf8;

static const utf8_ch UTF8_NULL = {0};

utf8_ch utf8Get(int fd);
utf8_ch utf8Decomp(utf8 *);
utf8_ch utf8Code32(uint32_t);

void utf8Put(utf8_ch);

int utf8Equal(utf8_ch, utf8_ch);
int utf8Next(utf8 **);

#endif // UTF8_HELPER
