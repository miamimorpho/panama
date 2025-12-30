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
#include "json_wrapper.h"

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

const char *colorname[COLOR_COUNT] = {
	[COLOR_BG] = "bg",
	[COLOR_FG] = "fg",
	[COLOR_MONSTER] = "monster",
	[COLOR_ITEM] = "item",
};

typedef void (*ColorJsonFn)(json_value *, const char *, Color *);
typedef int (*ColorCompareFn)(const Color *, const Color *);
typedef void (*ColorFgBrushFn)(const Color *);
typedef void (*ColorBgBrushFn)(const Color *);

const char *g_term_error = {0};

struct Term {
	struct termios initial_termios;
	struct {
		ColorCompareFn compare;
		ColorFgBrushFn front;
		ColorBgBrushFn back;
	} color;
	ColorPalette palette;
	uint16_t width;
	uint16_t height;
	int resize;
	int frame;
	struct TermTile fb[2 * FRAMEBUFFER_SIZE];
};
static struct Term *TERM;

#define NEXT_FRAME ((TERM->frame + 1) % 2)

static void
colorMonoJson(json_value *root, const char *key, Color *c)
{
	const long long *i = jsonGetInt(root, key);
	if (i) {
		c->mono = i ? true : false;
	}
}

static int
colorMonoCompare(const Color *a, const Color *b)
{
	return a->mono == b->mono;
}

static void
colorMonoFg(const Color *c)
{
	printf("\x1b[%sm", c->mono ? "7" : "27");
	// 1 → "\x1b[7m"   (invert: fg↔bg swap)
	// 0 → "\x1b[27m"  (normal: reset invert)
}
static void
colorMonoBg(const Color *c)
{
	(void) (c);
}

static void
colorAnsi16Json(json_value *root, const char *key, Color *c)
{
	const long long *i = jsonGetInt(root, key);
	if (i || *i < 0 || *i > 16) {
		c->ansi16 = *i;
	} else {
		fprintf(
			stderr,
			"WARN: %s color invalid: values must be 0 <= int < 16 for ansi16\n",
			key);
		c->ansi16 = 1;
	}
}
static int
colorAnsi16Compare(const Color *a, const Color *b)
{
	return a->ansi16 == b->ansi16;
}
// ANSI16: direct mapping 30-37 (0-7) + 90-97 (8-15)
static void
colorAnsi16Fg(const Color *c)
{
	int code = (c->ansi16 < 8) ? 30 + c->ansi16 : 90 + (c->ansi16 - 8);
	printf("\x1b[38;%dm", code);
}
static void
colorAnsi16Bg(const Color *c)
{
	int code = (c->ansi16 < 8) ? 30 + c->ansi16 : 90 + (c->ansi16 - 8);
	printf("\x1b[48;%dm", code);
}

static int
colorAnsi256Compare(const Color *a, const Color *b)
{
	return a->ansi256 == b->ansi256;
}
static void
colorAnsi256Fg(const Color *c)
{
	printf("\x1b[38;5;%um", c->ansi256);
}
static void
colorAnsi256Bg(const Color *c)
{
	printf("\x1b[48;5;%um", c->ansi256);
}

static int
colorTrueCompare(const Color *a, const Color *b)
{
	return (a->rgb.r == b->rgb.r) && (a->rgb.g == b->rgb.g) &&
		   (a->rgb.b == b->rgb.b);
}
static void
colorTrueFg(const Color *c)
{
	printf("\x1b[38;2;%u;%u;%um", c->rgb.r, c->rgb.g, c->rgb.b);
}
static void
colorTrueBg(const Color *c)
{
	printf("\x1b[48;2;%u;%u;%um", c->rgb.r, c->rgb.g, c->rgb.b);
}

struct TermUI
termRoot(void)
{
	return (struct TermUI) {0, 0, TERM->width, TERM->height,
							0, 0, COLOR_BG,	   COLOR_FG};
}

struct TermUI
termWin(struct TermUI cur, int width, int height, int margin_left,
		int margin_top)
{
	return (struct TermUI) {cur.margin_top + margin_top + cur.y,
							cur.margin_left + margin_left + cur.x,
							width,
							height,
							cur.x,
							cur.y,
							cur.fg,
							cur.bg};
}

static inline struct TermTile *
fbGet(int frame, uint16_t x, uint16_t y)
{
	size_t index = (FRAMEBUFFER_SIZE * frame) + (y * TERM->width + x);
	return &TERM->fb[index];
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

	struct TermTile *tile =
		fbGet(TERM->frame, ui->x + ui->margin_left, ui->y + ui->margin_top);
	tile->utf = ch;
	tile->fg = TERM->palette[ui->fg];
	tile->bg = TERM->palette[ui->bg];
	termMove(ui, ui->x + 1, ui->y);
}

void
termPuts(struct TermUI *ui, utf8 *str)
{
	if (!str)
		return;

	do {
		utf8_ch ch = utf8Decomp(str);
		termPut(ui, ch);
	} while (0 == utf8Next(&str));
}

/* reads in an utf8 enocoded cha r, if its size is greater than 1 byte,
 * its value must be greater than 255, and therefore not an ascii
 * character at all, return 0
 */
static char
asciiGet(void)
{
	utf8_ch utf = utf8Get(STDIN_FILENO);
	if (utf.size != 1)
		return 0;

	return utf.bytes[0];
}

/* reads in a sanitized and unmalformed ascii char
 * if its ESC, break off into control seqeuence handling
 */
int64_t
termGet(void)
{
	if (TERM->resize == 1) {
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
}

void
termFlush(void)
{
	Color pen_fg = {0};
	TERM->color.front(&pen_fg);
	Color pen_bg = {1};
	TERM->color.back(&pen_bg);

	for (uint16_t y = 0; y < TERM->height; y++) {
		uint16_t x = 0;
		bool pen_down = false;

		while (x < TERM->width) {

			struct TermTile *cur = fbGet(TERM->frame, x, y);
			struct TermTile *nex = fbGet(NEXT_FRAME, x, y);
			if (utf8Equal(cur->utf, nex->utf) ||
				!TERM->color.compare(&cur->fg, &nex->fg) ||
				!TERM->color.compare(&cur->bg, &nex->bg)) {

				if (!pen_down) {
					printf(ESCA "%d;%dH", y + 1, x + 1);
					pen_down = true;
				}

				if (!TERM->color.compare(&pen_fg, &cur->fg)) {
					TERM->color.front(&cur->fg);
					pen_fg = cur->fg;
				}
				if (!TERM->color.compare(&pen_bg, &cur->bg)) {
					TERM->color.back(&cur->bg);
					pen_bg = cur->bg;
				}

				utf8Put(cur->utf);

			} else {
				pen_down = false;
			}
			x++;
		}
	}

	if (TERM->resize) {
		resize();
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

static void
exitHandler(int i)
{
	(void) i;
	restore();

	if (g_term_error)
		printf("%s", g_term_error);

	exit(0);
}

static void
setup_signals(void)
{
	struct sigaction sa;

	// Common handler: exit
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = exitHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0; // no SA_RESTART etc. unless you want it

	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTRAP, &sa, NULL);
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);

	// SIGWINCH handler: resizeHandler
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = resizeHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0; // important if you want read() interrupted

	sigaction(SIGWINCH, &sa, NULL);
}

void
optionsPalette(json_value *palette, ColorJsonFn reader)
{
	reader(palette, "bg", &TERM->palette[COLOR_BG]);
	reader(palette, "fg", &TERM->palette[COLOR_FG]);
	reader(palette, "monster", &TERM->palette[COLOR_MONSTER]);
	reader(palette, "item", &TERM->palette[COLOR_ITEM]);
}

void
options(void)
{
	json_value *r = jsonReadFile("options/init");
	if (!r)
		return;

	const char *jpalettename = jsonGetString(r, "palette");
	json_value *jpalette = jsonReadFile(jpalettename);
	if (!jpalette) {
		fprintf(stderr, "FATAL: terminal.c:options() -> palette file listed in "
						"options/init.json not found\n");
		json_value_free(r);
		exit(1);
	}

	const char *color_mode = jsonGetString(jpalette, "color_mode");
	if (strncmp("mono", color_mode, strlen("mono")) == 0) {
		TERM->color.compare = colorMonoCompare;
		TERM->color.front = colorMonoFg;
		TERM->color.back = colorMonoBg;
		optionsPalette(jpalette, colorMonoJson);
	} else if (strncmp("ansi16", color_mode, strlen("mono")) == 0) {
		TERM->color.compare = colorAnsi16Compare;
		TERM->color.front = colorAnsi16Fg;
		TERM->color.back = colorAnsi16Bg;
		TERM->palette[COLOR_BG].mono = 1;
	} else if (strncmp("ansi256", color_mode, strlen("mono")) == 0) {
		TERM->color.compare = colorAnsi256Compare;
		TERM->color.front = colorAnsi256Fg;
		TERM->color.back = colorAnsi256Bg;
		TERM->palette[COLOR_BG].mono = 1;
	} else if (strncmp("true", color_mode, strlen("mono")) == 0) {
		TERM->color.compare = colorTrueCompare;
		TERM->color.front = colorTrueFg;
		TERM->color.back = colorTrueBg;
		TERM->palette[COLOR_BG].mono = 1;
	} else {
		exit(1);
	}

	json_value_free(jpalette);
	json_value_free(r);
}

void
termInit(void)
{
	TERM = malloc(sizeof(struct Term));
	*TERM = (struct Term) {0};

	options();

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
	// atexit(restore);
	setup_signals();

	SAY(ESCA ALTBUF HIGH ESCA CLEARTERM ESCA CURSOR LOW);

	resizeHandler(0);
	resize();
	TERM->resize = 0;
}

void
termClose(void)
{
	restore();
	free(TERM);
}

void
termSize(uint16_t *width, uint16_t *height)
{
	*width = TERM->width;
	*height = TERM->height;
}
