#include <unistd.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>

#include "terminal.h"
#include "arr.h"

#define ALTBUF "?1049"
#define CURSOR "?25"
#define HIGH "h"
#define LOW "l"
#define CLEARTERM "2J"
#define ESC "\x1b"
#define ESCA ESC "["

#define SAY(s) write(STDOUT_FILENO, s, sizeof(s))
#define DOES_PRINT(c_) ((c_) >= 0x20 && (c_) <= 0x7E)
// #define DOES_TERMINATE(c_) ((c_) >= 0x40 && (c_) <= 0x7E)
#define NEXT_FRAME(term_) ((term_.frame_x + 1) % 2)

struct Term {
	struct termios initial_termios;
	uint16_t width;
	uint16_t height;
	int resize;
	int frame_x;
	struct TermTile *buffer[2];
};
static struct Term term_g;

void
termPut(struct TermUI *ui, utf8_ch ch)
{
	if (ui->y < 0 || ui->y >= term_g.height)
		return;

	if (ui->x >= term_g.width) {
		ui->y += ui->x / term_g.width;
		ui->x %= term_g.width; // use modulo instead of resetting to 0
		if (ui->y >= term_g.height)
			return; // check after adjustment
	}

	if (ui->x < 0)
		return;

	size_t index = (size_t) ui->y * term_g.width + ui->x;
	struct TermTile *buffer = term_g.buffer[term_g.frame_x];

	buffer[index].utf = ch;

	ui->x++;
}

void
termPuts(struct TermUI *ui, const char *str)
{
	if (!str)
		return;

	while (*str != '\0') {
		utf8_ch ch = utf8Char(str);
		termPut(ui, ch);
		if (utf8Next(&str))
			break;
	}
}

static char
asciiGet(void)
{
	utf8_ch utf = utf8Get(STDIN_FILENO);
	if ((utf8ChSize(utf.bytes) != 1))
		return 0;

	return utf.bytes[0];
}

int64_t
termGet(void)
{
	char first = asciiGet();
	if (first == 0)
		return T_KEY_NONE;

	if (first == '\x1b') {
		char second = asciiGet();
		if (second == 0)
			return T_KEY_DEFORMED;
		if (second == '[') {
			char third = asciiGet();
			switch (third) {
			case '0':
				return T_KEY_DEFORMED;
			case 'A':
				return T_KEY_UP;
			case 'B':
				return T_KEY_DOWN;
			case 'C':
				return T_KEY_RIGHT;
			case 'D':
				return T_KEY_LEFT;
			default:
				return T_KEY_NAUGHTY;
			}
		}
		return T_KEY_NAUGHTY;
	}

	if (DOES_PRINT(first)) {
		return first;
	}

	return T_KEY_NAUGHTY;
}

void
termClear(void)
{
	size_t size =
		arrOverflowCheck(term_g.width * term_g.height, sizeof(struct TermTile));
	memset(term_g.buffer[term_g.frame_x], 0, size);
}

static void *
framebufferCreate(size_t width, size_t height)
{
	size_t size;
	return arrMalloc(width * height, sizeof(struct TermTile), &size);
}

static void
resizeMalloc(void)
{
	struct winsize ws;
	ioctl(1, TIOCGWINSZ, &ws);
	term_g.width = ws.ws_col;
	term_g.height = ws.ws_row;
	SAY(ESCA CLEARTERM);

	for (int i = 0; i < 2; i++) {
		term_g.buffer[i] = framebufferCreate(term_g.width, term_g.height);
	}

	term_g.resize = 0;
}

void
termRefresh(void)
{
	if (term_g.resize) {
		resizeMalloc();
	}

	struct TermTile *prev_frame = term_g.buffer[NEXT_FRAME(term_g)];
	struct TermTile *frame = term_g.buffer[term_g.frame_x];

	for (uint16_t y = 0; y < term_g.height; y++) {
		uint16_t x = 0;
		while (x < term_g.width) {
			size_t i = y * term_g.width + x;
			if (!utf8Equal(frame[i].utf, prev_frame[i].utf)) {

				printf(ESCA "%d;%dH", y + 1, x + 1);

				while (x < term_g.width) {
					i = y * term_g.width + x;
					if (!utf8Equal(frame[i].utf, prev_frame[i].utf)) {
						utf8Put(frame[i].utf);
						x++;
					} else {
						break;
					}
				}

			} else {
				x++;
			}

		} // end of line
	}

	term_g.frame_x = NEXT_FRAME(term_g);
	termClear();
}

static void
restore(void)
{
	SAY(ESCA CLEARTERM ESCA CURSOR HIGH ESCA ALTBUF LOW);

	// restore original termios params
	tcsetattr(1, TCSANOW, &term_g.initial_termios);
}

static void
resizeHandler(int i)
{
	(void) (i);
	term_g.resize = 1;
}

void
termInit(void)
{
	setlocale(LC_ALL, "");

	// disable output buffering, so chars will flush
	// immediately instead of after \n
	setvbuf(stdout, NULL, _IONBF, 0);

	// disable input buffering similarly (canonical/unix mode)
	// along with input echoing
	struct termios t;
	tcgetattr(1, &t);
	term_g.initial_termios = t;
	t.c_lflag &= (~ECHO & ~ICANON);

	// make sure CPU blocks on input wait to reduce usage
	t.c_cc[VMIN] = 1;  // Wait for at least 1 byte on input
	t.c_cc[VTIME] = 1; // 1sec timeout on input
	tcsetattr(1, TCSANOW, &t);

	// assign handlers for polite exit and resize
	atexit(restore);
	signal(SIGTERM, exit);
	signal(SIGINT, exit);
	signal(SIGTRAP, exit);
	signal(SIGWINCH, resizeHandler);

	SAY(ESCA ALTBUF HIGH ESCA CLEARTERM ESCA CURSOR LOW);

	resizeHandler(0);
	resizeMalloc();
}

void
termSize(uint16_t *width, uint16_t *height)
{
	*width = term_g.width;
	*height = term_g.height;
}
