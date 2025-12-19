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
// #include "maths.h"

#define TERMINAL_DEF_WID 80
#define TERMINAL_DEF_HEI 24
#define FRAMEBUFFER_WID (TERMINAL_DEF_WID * 3)
#define FRAMEBUFFER_HEI (TERMINAL_DEF_HEI * 3)
#define FRAMEBUFFER_SIZE (FRAMEBUFFER_WID * FRAMEBUFFER_HEI)

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

struct Term {
	struct termios initial_termios;
	uint16_t width;
	uint16_t height;
	int resize;
	int frame;
	struct TermTile fb[2 * FRAMEBUFFER_SIZE];
};
static struct Term *TERM;

#define NEXT_FRAME ((TERM->frame + 1) % 2)

struct TermUI
termRoot(void)
{
	return (struct TermUI) {
		0, 0, TERM->width, TERM->height, 0, 0,
	};
}

struct TermUI
termWin(struct TermUI cur, int width, int height, int margin_left,
		int margin_top)
{

	return (struct TermUI) {
		cur.margin_top + margin_top + cur.y,
		cur.margin_left + margin_left + cur.x,
		width,
		height,
		cur.x,
		cur.y,
	};
}

static inline struct TermTile *
fbGet(int frame, uint16_t x, uint16_t y)
{
	size_t index = (FRAMEBUFFER_SIZE * frame) + (y * TERM->width + x);
	return &TERM->fb[index];
}

int
fbCompare(uint16_t x, uint16_t y)
{
	struct TermTile *cur = fbGet(TERM->frame, x, y);
	struct TermTile *nex = fbGet(NEXT_FRAME, x, y);
	return (utf8Equal(cur->utf, nex->utf));
}

void
termMove(struct TermUI *ui, int dx, int dy)
{
	ui->x = dx;
	ui->y = dy;

	if (ui->x >= ui->width) {
		ui->y += ui->x / ui->width;
		ui->x %= ui->width;
		if (ui->y >= TERM->height)
			return; // check after adjustment
	}
}

void
termPut(struct TermUI *ui, utf8_ch ch)
{

	if (ui->x < 0 || ui->y < 0)
		return;
	if (ui->x >= TERM->width)
		return;

	// if(utf8Equal(ch, UTF8_NULL))
	//     ch = utf8Char(" ");

	fbGet(TERM->frame, ui->x + ui->margin_left, ui->y + ui->margin_top)->utf =
		ch;
	termMove(ui, ui->x + 1, ui->y);
}

void
termPuts(struct TermUI *ui, const char *str)
{
	if (!str)
		return;

	do {
		utf8_ch ch = utf8Char(str);
		termPut(ui, ch);
	} while (0 == utf8Next(&str));
}

/* reads in an utf8 enocoded char, if its size is greater than 1 byte,
 * its value must be greater than 255, and therefore not an ascii
 * character at all, return 0
 */
static char
asciiGet(void)
{
	utf8_ch utf = utf8Get(STDIN_FILENO);
	if ((utf8ChSize(utf.bytes) != 1))
		return 0;

	return utf.bytes[0];
}

/* reads in a sanitized and unmalformed ascii char
 * if its ESC, break off into control seqeuence handling
 */
int64_t
termGet(void)
{
	if (TERM->resize == 1){
	  TERM->resize = 0;
	  return T_KEY_NONE;
	}
	
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

static void
clear(int frame)
{
	struct TermTile *start = TERM->fb + (frame * FRAMEBUFFER_SIZE);
	memset(start, 0, FRAMEBUFFER_SIZE);
}

void
termClear(void)
{
	clear(TERM->frame);
}

static void
resize(void)
{
	struct winsize ws;
	ioctl(1, TIOCGWINSZ, &ws);
	TERM->width = ws.ws_col;
	TERM->height = ws.ws_row;
	SAY(ESCA CLEARTERM);
	termClear();
	// clear(1);
	//TERM->resize = 0;
}

void
termFlush(void)
{
	int next = NEXT_FRAME;

	for (uint16_t y = 0; y < TERM->height; y++) {
		uint16_t x = 0;
		while (x < TERM->width) {
			if (!fbCompare(x, y)) {
				printf(ESCA "%d;%dH", y + 1, x + 1);
				while (x < TERM->width) {
					if (!fbCompare(x, y)) {
						utf8_ch ch = fbGet(TERM->frame, x, y)->utf;
						utf8Put(ch);
						fbGet(next, x, y)->utf = ch;
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

	if (TERM->resize) {
		resize();
		//TERM->resize = 1;
	}
	
	TERM->frame = NEXT_FRAME;
}

static void
restore(void)
{
	SAY(ESCA CLEARTERM ESCA CURSOR HIGH ESCA ALTBUF LOW);

	// restore original termios params
	tcsetattr(1, TCSANOW, &TERM->initial_termios);
}

static void
resizeHandler(int i)
{
	(void) (i);
	TERM->resize = 1;
}

static void setup_signals(void) {
    struct sigaction sa;

    // Common handler: exit
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = exit;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;                  // no SA_RESTART etc. unless you want it

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTRAP, &sa, NULL);

    // SIGWINCH handler: resizeHandler
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = resizeHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;                  // important if you want read() interrupted

    sigaction(SIGWINCH, &sa, NULL);
}

void
termInit(void)
{
	TERM = malloc(sizeof(struct Term));

	setlocale(LC_ALL, "");

	// disable output buffering, so chars will flush
	// immediately instead of after \n
	setvbuf(stdout, NULL, _IONBF, 0);

	// disable input buffering similarly (canonical/unix mode)
	// along with input echoing
	struct termios t;
	tcgetattr(1, &t);
	TERM->initial_termios = t;
	t.c_lflag &= (~ECHO & ~ICANON);

	// make sure CPU blocks on input wait to reduce usage
	t.c_cc[VMIN] = 1;  // Wait for at least 1 byte on input
	t.c_cc[VTIME] = 1; // 1sec timeout on input
	tcsetattr(1, TCSANOW, &t);

	// assign handlers for polite exit and resize
	atexit(restore);
	setup_signals();
	
	//signal(SIGTERM, exit);
	//signal(SIGINT, exit);
	//signal(SIGTRAP, exit);
	//signal(SIGWINCH, resizeHandler);

	SAY(ESCA ALTBUF HIGH ESCA CLEARTERM ESCA CURSOR LOW);

	resizeHandler(0);
	resize();
	TERM->resize = 0;
	
}

void
termSize(uint16_t *width, uint16_t *height)
{
	*width = TERM->width;
	*height = TERM->height;
}
