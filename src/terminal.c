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
#include "maths.h"

/* Output focused ANSI escape sequences */
#define      with ";"
#define     plain "0" /* or "" */
#define        no "2"
#define    bright "1"
#define       dim "2"
#define    italic "3"
#define underline "4"
#define   reverse "7"

#define        Tfg "3"
#define        Tbg "4"
#define     br_fg "9"
#define     br_bg "10"
#define     black "0"
#define       red "1"
#define     green "2"
#define    yellow "3"
#define      blue "4"
#define   magenta "5"
#define      cyan "6"
#define     white "7"

#define    alt_buf "?1049"
#define       curs "?25"
#define term_clear "2J"
#define clear_line "2K"
#define       high "h"
#define        low "l"
#define       jump "H"

#define esc "\x1b"
#define esca esc "["
#define wfg "38;5;"
#define wbg "48;5;"
#define color "m"
#define fmt(f) esca f "m"

#define SAY(s) write(1,s,sizeof(s))
#define sz(s) (sizeof(s)/sizeof(*s))
#define DOES_PRINT(c_) ((c_) >= 0x20 && (c_) <= 0x7E)
#define DOES_TERMINATE(c_) ((c_) >= 0x40 && (c_) <= 0x7E)
#define NEXT_FRAME(term_) ((term_.frame_x + 1) % 2)

struct TermTrueColor{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct TermChar{
    utf32_t ch;
    struct TermTrueColor fg;
    struct TermTrueColor bg;
};

struct Term{
    struct termios initial_termios;
    uint16_t width;
    uint16_t height;
    int resize;
    int frame_x;
    struct TermChar *buffer[2];
};
static struct Term term_g;

#define UTF8_1BYTE_PREFIX   0x00    // 0xxxxxxx
#define UTF8_2BYTE_PREFIX   0xC0    // 110xxxxx
#define UTF8_3BYTE_PREFIX   0xE0    // 1110xxxx  
#define UTF8_4BYTE_PREFIX   0xF0    // 11110xxx
#define UTF8_CONT_PREFIX    0x80    // 10xxxxxx
#define UTF8_6BIT_MASK      0x3F    // 00111111

static int utf32to8(utf32_t in, char* out) {
  if (in <= 0x7F) {
    out[0] = in;
    return 1;
  }

  if (in <= 0x7FF) {
    out[0] = UTF8_2BYTE_PREFIX | (in >> 6);
    out[1] = UTF8_CONT_PREFIX | (in & UTF8_6BIT_MASK);
    return 2;
  }

  if (in <= 0xFFFF) {
    out[0] = UTF8_3BYTE_PREFIX | (in >> 12);
    out[1] = UTF8_CONT_PREFIX | ((in >> 6) & UTF8_6BIT_MASK);
    out[2] = UTF8_CONT_PREFIX | (in & UTF8_6BIT_MASK);
    return 3;
  }

  out[0] = UTF8_4BYTE_PREFIX | (in >> 18);
  out[1] = UTF8_CONT_PREFIX | ((in >> 12) & UTF8_6BIT_MASK);
  out[2] = UTF8_CONT_PREFIX | ((in >> 6) & UTF8_6BIT_MASK);
  out[3] = UTF8_CONT_PREFIX | (in & UTF8_6BIT_MASK);
  return 4;
}

static inline void utf32write(utf32_t in){

    switch(in){
    case 0:
        in = (utf32_t)' ';
    }
    
    char out[4];
    int len = utf32to8(in, out);
    write(STDOUT_FILENO, out, len);
}

static inline char termGetCh(void){
    char ch;
    read(STDIN_FILENO, &ch, 1);
    return ch;
}

void termCh(struct TermUI ui, utf32_t ch){
    if(ui.x < 0 || 
            ui.x >= term_g.width ||
            ui.y < 0 ||
            ui.y >= term_g.height) return;
   
    struct TermChar *buffer = term_g.buffer[term_g.frame_x];
    buffer[ui.y * term_g.width + ui.x].ch = ch;
}

int64_t termIn(void){
    char buf[32];
    memset(buf, 0, sizeof(buf));
    buf[0] = termGetCh();
    if(buf[0] == 0) return 0;

    if (buf[0] == '\x1b'){
        int total = 1;
        while(total < 31){
            buf[total] = termGetCh();
            if(buf[total] == 0) break;

            if(buf[total] == '['){
                total++;
                continue;
            }

            if(DOES_TERMINATE(buf[total])){
                total++;
                break;
            }

        }

        if(total >= 3 && buf[1] == '['){
            switch (buf[2]) {
            case 'A': return T_KEY_UP;
            case 'B': return T_KEY_DOWN;
            case 'C': return T_KEY_RIGHT;
            case 'D': return T_KEY_LEFT;
            }
        }

        return T_KEY_ESCAPE;
    }

    if(DOES_PRINT(buf[0])){
        return buf[0];
    }

    return T_KEY_NONE;
}

void termClear(void){
    size_t size = 
        sizeOverflowCheck(term_g.width * term_g.height, sizeof(struct TermChar));
    memset(term_g.buffer[term_g.frame_x], 0, size);
}

static void resizeMalloc(void){

        struct winsize ws;
        ioctl(1, TIOCGWINSZ, &ws);
        term_g.width = ws.ws_col;
        term_g.height = ws.ws_row;
        SAY(esca term_clear);
        
        size_t size = 
            sizeOverflowCheck( term_g.width * term_g.height, sizeof(struct TermChar));
        term_g.buffer[0] = realloc(term_g.buffer[0], size);
        memset(term_g.buffer[0], 0, size);
        term_g.buffer[1] = realloc(term_g.buffer[1], size);
        memset(term_g.buffer[1], 0, size);
        term_g.resize = 0;
}

void termRefresh(void) {

    if(term_g.resize){
        resizeMalloc();
    }

    struct TermChar *prev_frame = term_g.buffer[NEXT_FRAME(term_g)];
    struct TermChar *frame = term_g.buffer[term_g.frame_x];

    for (uint16_t y = 0; y < term_g.height; y++) {
        uint16_t x = 0;
        while (x < term_g.width) {
            size_t i = y * term_g.width + x;
            if (frame[i].ch != prev_frame[i].ch) {
            
                printf(esca "%d;%dH", y + 1, x + 1);
                
                while (x < term_g.width) {
                    i = y * term_g.width + x;
                    if (frame[i].ch != prev_frame[i].ch) {
                        utf32write(frame[i].ch);
                        x++;
                    }else{
                        break;
                    }
                }
 
            } else {
                x++;
            }
            
        }// end of line
    }

    term_g.frame_x = NEXT_FRAME(term_g);
}

static void restore(void) {
	SAY(
    //enter alternate buffer if we haven't already
		esca alt_buf high
    //clean up the buffer
		esca term_clear
	//show the cursor
		esca curs high
	//return to the main buffer
		esca alt_buf low
	);

	//restore original termios params
	tcsetattr(1, TCSANOW, &term_g.initial_termios);
}

static void resizeHandler(int i) {
    term_g.resize = 1;
}

void termInit(void) {
 
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
	
    SAY (
    // switch to alternative buffer,
    // initial screen clear
		esca term_clear
    // hide cursor
		esca curs low
	);

    resizeHandler(0);
    resizeMalloc();
}

void termSize(uint16_t *width, uint16_t *height){
    *width = term_g.width;
    *height = term_g.height;
}
