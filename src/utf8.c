#include <string.h>
#include <unistd.h>

#include "utf8.h"

#define UTF8_2BYTE_PREFIX 0xC0 // 110xxxxx
#define UTF8_3BYTE_PREFIX 0xE0 // 1110xxxx
#define UTF8_4BYTE_PREFIX 0xF0 // 11110xxx
#define UTF8_CONT_PREFIX 0x80  // 10xxxxxx
#define UTF8_6BIT_MASK 0x3F	   // 00111111

utf8_ch
utf8Code(uint32_t code)
{
	utf8_ch out = {0};

	if (code <= 0x7F) {
		out.bytes[0] = code;
		return out;
	}

	if (code <= 0x7FF) {
		out.bytes[0] = UTF8_2BYTE_PREFIX | (code >> 6);
		out.bytes[1] = UTF8_CONT_PREFIX | (code & UTF8_6BIT_MASK);
		return out;
	}

	if (code <= 0xFFFF) {
		out.bytes[0] = UTF8_3BYTE_PREFIX | (code >> 12);
		out.bytes[1] = UTF8_CONT_PREFIX | ((code >> 6) & UTF8_6BIT_MASK);
		out.bytes[2] = UTF8_CONT_PREFIX | (code & UTF8_6BIT_MASK);
		return out;
	}

	out.bytes[0] = UTF8_4BYTE_PREFIX | (code >> 18);
	out.bytes[1] = UTF8_CONT_PREFIX | ((code >> 12) & UTF8_6BIT_MASK);
	out.bytes[2] = UTF8_CONT_PREFIX | ((code >> 6) & UTF8_6BIT_MASK);
	out.bytes[3] = UTF8_CONT_PREFIX | (code & UTF8_6BIT_MASK);
	return out;
}

// Determine UTF-8 character length from first byte
unsigned int
utf8ChSize(utf8_str s)
{
	unsigned char first = s[0];
	switch (first >> 4) {
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
		return 1; // 0xxxxxxx
	case 0xC:
	case 0xD:
		return 2; // 110xxxxx
	case 0xE:
		return 3; // 1110xxxx
	case 0xF:
		// Check if it's 11110xxx (4-byte) or 1111xxxx (invalid)
		return (first & 0x08) ? 0 : 4;
	default:
		return 0; // 10xxxxxx or invalid
	}
}

void
utf8Put(utf8_ch ch)
{
    //if(utf8Equal(ch, UTF8_NULL)){
    //    write(STDOUT_FILENO, " ", 1);
    //    return;
    //}

	int len = utf8ChSize(ch.bytes);
	write(STDOUT_FILENO, ch.bytes, len);
}

int
utf8Equal(utf8_ch a, utf8_ch b)
{
	return !(memcmp(&a, &b, sizeof(utf8_ch)));
}

utf8_ch
utf8Char(utf8_str in)
{
	utf8_ch out;
	int len = utf8ChSize(in);
	memcpy(out.bytes, in, len);
	memset(out.bytes + len, 0, 4 - len);
	return out;
}

int
utf8Next(utf8_str *str)
{
	if (!*str || **str == '\0') {
		return 1; // End of string
	}

	int len = utf8ChSize(*str);
	if (!len)
		return 1;
	*str += len;

	return **str == '\0' ? 1 : 0;
}

utf8_ch
utf8Get(int fd)
{
	utf8_ch ch = {0};
	int len = 0;
	if (read(fd, ch.bytes, 1) <= 0)
		return UTF8_NULL;

	if ((len = utf8ChSize(ch.bytes)) <= 1)
		return ch;

	if (read(fd, &ch.bytes[1], len - 1) != len - 1)
		return UTF8_NULL;

	// Fast continuation byte check - validate all at once
	// Each continuation byte must be 10xxxxxx
	unsigned char mask = 0;
	for (int i = 1; i < len; i++) {
		mask |= ch.bytes[i];
	}
	// If any continuation byte is invalid
	// mask will have wrong bits set
	if ((mask & 0xC0) != 0x80) {
		return UTF8_NULL;
	}

	return ch;
}
