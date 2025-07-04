/* See LICENSE file for license details */
/* llock - logical x display locker */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <crypt.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>

#include "config.h"

#define version "0.2"

static xcb_connection_t *c;
static xcb_key_symbols_t *k;
static xcb_screen_t *s;
static xcb_window_t w;
static char in[256] = {0};
static size_t inlen = 0;

const char* yescrypt_salt(void);
int verify_passwd(const char *input);
void hidecur(void);
void chcolor(uint32_t color);
void handle(xcb_key_press_event_t *e);

const char* yescrypt_salt(void){
	return crypt_gensalt("$y$", 0, NULL, 0);
}

int verify_passwd(const char *input){
	const char *hashed = crypt(input, passwd);
	return hashed && strcmp(hashed, passwd) == 0;
}	

void hidecur(){
	xcb_pixmap_t pm = xcb_generate_id(c);
	xcb_create_pixmap(c, 1, pm, w, 1, 1);
	xcb_cursor_t cur = xcb_generate_id(c);
	xcb_create_cursor(c, cur, pm, pm, 0, 0, 0, 0, 0, 0, 1, 1);
	xcb_change_window_attributes(c, w, XCB_CW_CURSOR, &cur);
	xcb_free_pixmap(c, pm);
	xcb_free_cursor(c, cur);
}

void chcolor(uint32_t color){
	xcb_change_window_attributes(c, w, XCB_CW_BACK_PIXEL, &color);
	xcb_clear_area(c, 0, w, 0, 0, 0, 0);
	xcb_flush(c);
}

void handle(xcb_key_press_event_t *e){
	xcb_keysym_t sym = xcb_key_press_lookup_keysym(k, e, 0);
	chcolor(cactive);
	if(sym == XK_Return){
			if(verify_passwd(in)){
			xcb_ungrab_keyboard(c, XCB_CURRENT_TIME);
			exit(0);
		}

		memset(in, 0, sizeof(in));
		inlen = 0;
	} else if(sym == XK_BackSpace && inlen > 0){
		in[--inlen] = '\0';
	} else if(inlen < sizeof(in)-1 && isprint(sym)){
		in[inlen++] = (char)sym;
	}
}

int main(int argc, char **argv){
	if(argc > 1){
		if(strcmp(argv[1], "-v") == 0){
			write(1, "lock-" version "\n", sizeof("lock-" version "\n") - 1);
			return 0;
		}

		if(strcmp(argv[1], "-h") == 0){
			write(1, "usage: ", 7);
			write(1, argv[0], strlen(argv[0]));
			write(1, " [options]..\n", 14);
			return 0;
		}
	}

	c = xcb_connect(NULL, NULL);
	k = xcb_key_symbols_alloc(c);
	s = xcb_setup_roots_iterator(xcb_get_setup(c)).data;
	w = xcb_generate_id(c);
	uint32_t m = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT;
	uint32_t vl[] = {cnormal, 1};
	xcb_create_window(c, XCB_COPY_FROM_PARENT, w, s->root,
		0, 0, s->width_in_pixels, s->height_in_pixels,
		0, XCB_WINDOW_CLASS_INPUT_OUTPUT, s->root_visual,
		m, vl);

	hidecur();
	xcb_map_window(c, w);
	xcb_grab_keyboard(c, 1, w, XCB_CURRENT_TIME, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	chcolor(cnormal);
	xcb_generic_event_t *e;
	while((e = xcb_wait_for_event(c))){
		if((e->response_type & ~0x80) == XCB_KEY_PRESS)
			handle((xcb_key_press_event_t*)e);
		free(e);
	}

	xcb_key_symbols_free(k);
	xcb_disconnect(c);

	return 0;
}
