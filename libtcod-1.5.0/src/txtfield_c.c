#include "txtfield.h"
#include <stdlib.h>
#include <string.h>

#define MAX_INT 0x7FFFFFFF

typedef struct {
    int x, y; /* coordinates on parent console */
    int w, h; /* textfield display size */
    int max; /* maximum nb of characters */
    int interval; /* cursor blinking interval */
    int halfinterval; /* half of the above */
    int ascii_cursor; /* cursor char. 0 if none */
	int cursor_pos, sel_start,sel_end; /* cursor position in text, selection range */
	int tab_size; /* tab size, if 0, no tab */
    char * prompt; /* prompt to be displayed before the string */
	int textx,texty; /* coordinate of start of text (after prompt) */
    TCOD_console_t con; /* offscreen console that will contain the textfield */
    bool input_continue; /* controls whether ENTER has been pressed */
    int len; /* allocated size of the text */
    int curlen; /* current length of the text */
    TCOD_color_t back; /* background colour */
    TCOD_color_t fore; /* foreground colour */
    float transparency; /* background transparency */
	bool multiline; /* multiline support ? */
    char * text; /* the text itself */
} text_t;

/* ctor */
TCOD_text_t TCOD_text_init (int x, int y, int w, int h, int max_chars) {
    text_t * data = (text_t*)calloc(sizeof(text_t),1);
    data->x = x;
    data->y = y;
    data->w = MAX(w,1); /* no zeroes! */
    data->h = MAX(h,1);
    data->multiline = (h > 1);
    data->max = (max_chars > 0 ? max_chars + 1 : MAX_INT);
    data->interval = 800;
    data->halfinterval = 400;
    data->ascii_cursor = 0;
    data->prompt = NULL;
    data->textx = data->texty = 0;
    data->con = TCOD_console_new(w,h);
	data->sel_start = MAX_INT;
	data->sel_end = -1;
//	if (! data->multiline ) {
//		data->max = MIN(w - data->textx,data->max);
//	} else {
//		data->max = MIN(w*(h-data->texty) - data->textx,data->max);
//	}
    if (max_chars && max_chars > 0) data->max = max_chars;
    else data->max = data->w * data->h;
    data->input_continue = true;
    data->len = MIN(64,data->max);
    data->text = (char*)calloc(data->len,sizeof(char));
    data->back.r = data->back.g = data->back.b = 0;
    data->fore.r = data->fore.g = data->fore.b = 255;
    data->transparency = 1.0f;
    return (TCOD_text_t)data;
}

/* set cursor and prompt */
void TCOD_text_set_properties (TCOD_text_t txt, int cursor_char, int blink_interval, char * prompt, int tab_size) {
    text_t * data = (text_t*)txt;
    data->interval = blink_interval;
    data->halfinterval = (blink_interval > 0 ? blink_interval / 2 : 0);
    data->ascii_cursor = cursor_char;
    if (data->prompt) free(data->prompt);
    data->prompt = prompt ? strdup(prompt) : NULL;
    data->textx = data->texty = 0;
	data->tab_size=tab_size;
    if ( prompt ) {
		char *ptr=prompt;
		while (*ptr) {
			data->textx++;
			if ( *ptr == '\n' || data->textx == data->w) {
				data->textx=0;data->texty++;
			}
			ptr++;
		}
	}
}

/* set colours */
void TCOD_text_set_colors (TCOD_text_t txt, TCOD_color_t fore, TCOD_color_t back, float back_transparency) {
    text_t * data = (text_t*)txt;
    data->back = back;
    data->fore = fore;
    data->transparency = back_transparency;
}

/* increase the buffer size. internal function */
static void allocate(text_t *data) {
    char *tmp;
    data->len *= 2;
    tmp = (char*)calloc(data->len,sizeof(char));
    memcpy(tmp,data->text,data->len/2);
    free(data->text);
    data->text = tmp;
}

/* insert a character at cursor position. internal function */
static void insertChar(text_t *data, char c) {
	char *ptr, *end;
	if (data->curlen + 1 == data->max) {
		/* max size reached. replace the last char. don't increase text size */
		*(data->text + data->curlen -1) = c;
		return;
	}
	if (data->curlen + 1 == data->len ) allocate(data);
	ptr=data->text + data->cursor_pos;
	end=data->text + data->curlen;
	do {
		*(end+1) = *end;
		end--;
	} while ( end >= ptr );
	*ptr = c;
	data->curlen++;
	data->cursor_pos++;
}

/* delete character at cursor position */
static void deleteChar(text_t *data) {
	char *ptr;
	if ( data->cursor_pos == 0 ) return;
	ptr=data->text + data->cursor_pos-1;
	do {
		*ptr = *(ptr+1);
		ptr++;
	} while (*ptr);
	if ( data->cursor_pos > 0 ) {
		data->cursor_pos--;
		data->curlen--;
	}
}

/* convert current cursor_pos into console coordinates. internal function */
static void get_cursor_coords(text_t *data, int *cx, int *cy) {
	char *ptr;
	if (data->multiline) {
		int curcount=data->cursor_pos;
		ptr=data->text;
		*cx = data->textx;
		*cy = data->texty;
		while (curcount > 0 && *ptr) {
			if ( *ptr == '\n') {
				*cx=0;
				(*cy)++;
			} else {
				(*cx)++;
				if ( *cx == data->w ) {
					*cx=0;
					(*cy)++;
				}
			}
			ptr++;
			curcount--;
		}
	} else {
		*cx = data->textx + data->cursor_pos;
		*cy = data->texty;
	}
}

/* set cursor_pos from coordinates. internal function */
static void set_cursor_pos(text_t *data, int cx, int cy, bool clamp) {
	if ( data->multiline ) {
		int curx=data->textx,cury=data->texty;
		char *ptr=data->text;
		int newpos=0;
		if ( clamp ) {
			cy=MAX(data->texty,cy);
			if ( cy == data->texty) cx = MAX(data->textx,cx);
		}
		// find the right line
		while ( *ptr && cury < cy ) {
			if (*ptr == '\n' || curx == data->w-1) {
				curx=0;cury++;
			} else curx++;
			ptr++;
			newpos++;
		}
		if ( ! clamp && cury != cy ) return;
		// check if cx can be reached
		while ( *ptr && curx < cx && *ptr != '\n') {
			ptr++;
			curx++;
			newpos++;
		}
		data->cursor_pos = newpos;
	} else {
		int newpos = cx - data->textx + (cy - data->texty)*data->w;
		if ( clamp ) newpos = CLAMP(0,data->curlen,newpos);
		if ( newpos >= 0 && newpos <= data->curlen ) data->cursor_pos = newpos;
	}
}


/* decreases the selection range start */
static void selectStart(text_t *data, int oldpos, TCOD_key_t key) {
	if ( data->multiline && data->cursor_pos != oldpos ) {
		if ( key.shift ) {
			if ( data->sel_start > data->cursor_pos ) data->sel_start = data->cursor_pos;
			else data->sel_end = data->cursor_pos;
		} else {
			data->sel_start=MAX_INT;
			data->sel_end=-1;
		}
	}
}

/* increases the selection range end */
static void selectEnd(text_t *data, int oldpos, TCOD_key_t key) {
	if ( data->multiline && data->cursor_pos != oldpos ) {
		if ( key.shift ) {
			if ( data->sel_end < data->cursor_pos ) data->sel_end = data->cursor_pos;
			else data->sel_start = data->cursor_pos;
		} else {
			data->sel_start=MAX_INT;
			data->sel_end=-1;
		}
	}
}

enum { TYPE_SYMBOL, TYPE_ALPHANUM, TYPE_SPACE };
static const char symbols[]="&~\"#'{([-|`_\\^@)]=+}*/!:;.,?<>";

/* check whether a character is a space */
/* this is needed because cctype isspace() returns rubbish for many diacritics */
bool is_space (int ch) {
    bool ret;
    switch (ch) {
        case ' ': case '\n': case '\r': case '\t': ret = true; break;
        default: ret = false; break;
    }
    return ret;
}

void typecheck (int * type, int ch) {
    if (strchr(symbols,ch)) *type = TYPE_SYMBOL;
    else if (is_space(ch)) *type = TYPE_SPACE;
    else *type = TYPE_ALPHANUM;
}

/* go one word left */
static void previous_word(text_t *data) {
	/* current character type */
	if ( data->cursor_pos > 0 ) {
		/* detect current char type (alphanum/space or symbol) */
		char *ptr=data->text + data->cursor_pos - 1;
		int curtype, prevtype;
		typecheck(&curtype,*ptr);
		/* go back until char type changes from alphanumeric to something else */
		do {
			data->cursor_pos--;
			ptr--;
			prevtype = curtype;
			typecheck(&curtype,*ptr);
		} while ( data->cursor_pos > 0 && !(curtype != TYPE_ALPHANUM && prevtype == TYPE_ALPHANUM));
	}
}

/* go one word right */
static void next_word(text_t *data) {
   /* current character type */
	if ( data->text[data->cursor_pos] ) {
		/* detect current char type (alphanum/space or symbol) */
		char *ptr=data->text + data->cursor_pos;
		int curtype, prevtype;
		typecheck(&curtype,*ptr);
		/* go forth until char type changes from non alphanumeric to alphanumeric */
		do {
			data->cursor_pos++;
			ptr++;
			prevtype = curtype;
			typecheck(&curtype,*ptr);
		} while ( *ptr && !(curtype == TYPE_ALPHANUM  && prevtype != TYPE_ALPHANUM));
	}
}

/* update returns false if enter has been pressed, true otherwise */
bool TCOD_text_update (TCOD_text_t txt, TCOD_key_t key) {
	int cx,cy,oldpos;
    text_t * data = (text_t*)txt;
	oldpos = data->cursor_pos;
    /* process keyboard input */
    switch (key.vk) {
        case TCODK_BACKSPACE: /* get rid of the last character */
			if ( data->sel_start != MAX_INT ) {
				int count = data->sel_end-data->sel_start;
				data->cursor_pos = data->sel_start+1;
				while ( count > 0 ) {
					deleteChar(data);
					count--;
					data->cursor_pos++;
				}
				data->cursor_pos--;
				data->sel_start=MAX_INT;
				data->sel_end=-1;
			} else {
				deleteChar(data);
			}
            break;
		case TCODK_DELETE:
			if ( data->sel_start != MAX_INT ) {
				int count = data->sel_end-data->sel_start;
				data->cursor_pos = data->sel_start+1;
				while ( count > 0 ) {
					deleteChar(data);
					count--;
					data->cursor_pos++;
				}
				data->cursor_pos--;
				data->sel_start=MAX_INT;
				data->sel_end=-1;
			} else if ( data->text[data->cursor_pos] ) {
				data->cursor_pos++;
				deleteChar(data);
			}
			break;
		/* shift + arrow / home / end = selection */
		/* ctrl + arrow = word skipping. ctrl + shift + arrow = word selection */
		case TCODK_LEFT:
			if ( data->multiline && key.shift && data->sel_end == -1) {
				data->sel_end = data->cursor_pos;
			}
			if ( data->cursor_pos > 0 ) {
				if ( key.lctrl || key.rctrl ) {
					previous_word(data);
				} else data->cursor_pos--;
				selectStart(data,oldpos,key);
			}
			break;
		case TCODK_RIGHT:
			if ( data->multiline && key.shift && data->sel_start == MAX_INT ) {
				data->sel_start = data->cursor_pos;
			}
			if ( data->text[data->cursor_pos] ) {
				if ( key.lctrl || key.rctrl ) {
					next_word(data);
				} else data->cursor_pos++;
				selectEnd(data,oldpos,key);
			}
			break;
		case TCODK_UP :
			get_cursor_coords(data,&cx,&cy);
			if ( data->multiline && key.shift && data->sel_end == -1) {
				data->sel_end = data->cursor_pos;
			}
			set_cursor_pos(data,cx,cy-1,false);
			selectStart(data,oldpos,key);
			break;
		case TCODK_DOWN :
			get_cursor_coords(data,&cx,&cy);
			if ( data->multiline && key.shift && data->sel_start == MAX_INT ) {
				data->sel_start = data->cursor_pos;
			}
			set_cursor_pos(data,cx,cy+1,false);
			selectEnd(data,oldpos,key);
			break;
		case TCODK_HOME:
			get_cursor_coords(data,&cx,&cy);
			if ( data->multiline && key.shift && data->sel_end == -1) {
				data->sel_end = data->cursor_pos;
			}
			if ( key.lctrl || key.rctrl ) {
				set_cursor_pos(data,0,0,true);
			} else {
				set_cursor_pos(data,0,cy,true);
			}
			selectStart(data,oldpos,key);
			break;
		case TCODK_END:
			get_cursor_coords(data,&cx,&cy);
			if ( data->multiline && key.shift && data->sel_start == MAX_INT ) {
				data->sel_start = data->cursor_pos;
			}
			if ( key.lctrl || key.rctrl ) {
				set_cursor_pos(data,data->w,data->h,true);
			} else {
				set_cursor_pos(data,data->w-1,cy,true);
			}
			selectEnd(data,oldpos,key);
			break;
        case TCODK_ENTER: /* validate input */
        case TCODK_KPENTER:
			if ( data->multiline ) {
				insertChar(data,'\n');
			} else {
	            data->input_continue = false;
			}
            break;
		case TCODK_TAB :
			if (data->tab_size ) {
				int count=data->tab_size;
				while ( count > 0 ) {
					insertChar(data,' ');
					count--;
				}
			}
			break;
        default: { /* append a new character */
            if (key.c > 31) {
				insertChar(data,(char)(key.c));
            }
            break;
        }
    }
    return data->input_continue;
}

/* renders the textfield */
void TCOD_text_render (TCOD_console_t con, TCOD_text_t txt) {
    text_t * data = (text_t*)txt;
    uint32 time = TCOD_sys_elapsed_milli();
	bool cursor_on = (int)( time % data->interval ) > data->halfinterval;
	char back=0;
	int curx,cury,cursorx,cursory, curpos;
	char *ptr;
    TCOD_console_set_background_color(data->con, data->back);
    TCOD_console_set_foreground_color(data->con, data->fore);
    TCOD_console_clear(data->con);

	/* compute cursor position */
	get_cursor_coords(data,&cursorx,&cursory);

	if ( cursor_on && data->ascii_cursor) {
		/* save the character under cursor position */
		back = data->text[data->cursor_pos];
		data->text[data->cursor_pos] = data->ascii_cursor;
	}
	/* render prompt */
    if (data->prompt) TCOD_console_print_left_rect(data->con,0,0,data->w,data->h,TCOD_BKGND_SET,"%s",data->prompt);
	/* render text */
	curx=data->textx;
	cury=data->texty;
	ptr=data->text;
	curpos=0;
	while (*ptr) {
		if ( *ptr == '\n') {
			if ( (curx == 0 || curpos == 0 ) && curpos >= data->sel_start && curpos < data->sel_end ) {
				/* inverted colors for selected empty lines */
				TCOD_console_set_back(data->con, curx, cury, data->fore, TCOD_BKGND_SET);
				TCOD_console_set_fore(data->con, curx, cury, data->back);
			}
			curx=0;
			cury++;
		} else {
			if ( curpos >= data->sel_start && curpos < data->sel_end ) {
				/* inverted colors for selection */
				TCOD_console_set_back(data->con, curx, cury, data->fore, TCOD_BKGND_SET);
				TCOD_console_set_fore(data->con, curx, cury, data->back);
			}
			TCOD_console_set_char(data->con,curx,cury,*ptr);
			curx++;
			if ( curx == data->w ) {
				curx=0;
				cury++;
			}
		}
		ptr++;
		curpos++;
	}
	if ( cursor_on ) {
		if ( data->ascii_cursor) {
			/* restore the character under cursor */
			data->text[data->cursor_pos] = back;
		} else {
			/* invert colors at cursor position */
			TCOD_console_set_back(data->con,cursorx,cursory,data->fore,TCOD_BKGND_SET);
			TCOD_console_set_fore(data->con,cursorx,cursory,data->back);
		}
	} else if (! cursor_on && ! data->ascii_cursor && data->multiline ) {
		/* normal colors for cursor ( might be inside selection ) */
		TCOD_console_set_back(data->con,cursorx,cursory,data->back,TCOD_BKGND_SET);
		TCOD_console_set_fore(data->con,cursorx,cursory,data->fore);
	}
    TCOD_console_blit(data->con,0,0,data->w,data->h,con,data->x,data->y,1.0f,data->transparency);
}

/* returns the text currently stored in the textfield object */
char * TCOD_text_get (TCOD_text_t txt) {
    return ((text_t*)txt)->text;
}

/* resets the text initial state */
void TCOD_text_reset (TCOD_text_t txt) {
    text_t * data = (text_t*)txt;
    memset(data->text,'\0',data->len);
    data->curlen = 0;
    data->input_continue = true;
}

/* destructor */
void TCOD_text_delete (TCOD_text_t txt) {
    text_t * data = (text_t*)txt;
    free(data->text);
    free(data->prompt);
    TCOD_console_delete(data->con);
    free(data);
}
