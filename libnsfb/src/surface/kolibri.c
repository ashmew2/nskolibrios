/*
 * Copyright 2016 Nina Kalinina <ninacarrot@ya.ru>
 * Copyright 2016 Ashish Gupta <ashmew2@gmail.com>
 *
 * This file is a part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdbool.h>
#include <stdlib.h>

#include "libnsfb.h"
#include "libnsfb_event.h"
#include "libnsfb_plot.h"
#include "libnsfb_plot_util.h"

#include "nsfb.h"
#include "surface.h"
#include "palette.h"
#include "plot.h"
#include "cursor.h"

#define EVENT_REDRAW              0x00000001
#define EVENT_KEY                 0x00000002
#define EVENT_BUTTON              0x00000004
#define EVENT_END_REQUEST         0x00000008
#define EVENT_DESKTOP_BACK_DRAW   0x00000010
#define EVENT_MOUSE_CHANGE        0x00000020
#define EVENT_IPC  		  0x00000040

int kolibri_get_button_id(void) {
    uint16_t __ret;
    __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(17));
    if((__ret & 0xFF)==0) return (__ret>>8)&0xFF; else return -1;
}

int kolibri_wait_for_event(void) {
    uint32_t __ret;
    __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(10));
    return __ret;
}

int kolibri_get_pressed_key(void) {
    uint16_t __ret;
    __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(2));
    if(!(__ret & 0xFF)) return (__ret>>8)&0xFF; else return 0;
}

void kolibri_define_window(uint16_t x1,uint16_t y1,uint16_t xsize,uint16_t ysize,
			   uint32_t body_color,uint32_t grab_color,uint32_t frame_color) {
    uint32_t a,b;
    a=(x1<<16)|xsize;
    b=(y1<<16)|ysize;
    __asm__ __volatile__("int $0x40"::"a"(0),"b"(a),"c"(b),"d"(body_color),"S"(grab_color),
			 "D"(frame_color));
}

void kolibri_redraw_window(int status) {
    __asm__ __volatile__("int $0x40"::"a"(12),"b"(status));
}

unsigned char * pixels;

void kolibri_set__wanted_events(uint32_t ev) { 
    __asm__ __volatile__ ("int $0x40"::"a"(40),"b"(ev));
}

inline void f65_32bpp(unsigned x, unsigned y, unsigned w, unsigned h, char *d) {
    __asm__ __volatile__ ("pusha");
    __asm__ __volatile__ ("nop"::"D"(0), "c"(w*65536+h), "d"(x*65536+y), "b"(d));
    __asm__ __volatile__ ("xor %eax, %eax");
    __asm__ __volatile__ ("movl %eax, %ebp");
    __asm__ __volatile__ ("pushl $32");
    __asm__ __volatile__ ("popl %esi");
    __asm__ __volatile__ ("int $0x40"::"a"(65));
    __asm__ __volatile__ ("popa");
}

unsigned kol_mouse_posw() {
    unsigned error;
    __asm__ __volatile__ ("int $0x40":"=a"(error):"a"(37), "b"(1));
    return error;
}


unsigned kol_mouse_btn() {
    unsigned error;
    __asm__ __volatile__ ("int $0x40":"=a"(error):"a"(37), "b"(2));
    return error;
}

unsigned kol_mouse_scroll() {
    unsigned error;
    __asm__ __volatile__ ("int $0x40":"=a"(error):"a"(37), "b"(7));
    return error;
}

unsigned kol_wait_for_event_with_timeout(int timeout)	// timeout is in 1/100 seconds {
    unsigned event;
    __asm__ __volatile__ ("int $0x40":"=a"(event):"a"(23), "b"(timeout));
    return event;
}

unsigned kol_scancodes() {
    unsigned error;
    __asm__ __volatile__ ("int $0x40":"=a"(error):"a"(66), "b"(1), "c"(1));
    return error;
}

void kolibri_redraw(nsfb_t *nsfb){	
    f65_32bpp(0,0, nsfb->width, nsfb->height, pixels+1);
}

unsigned kol_skin_h() {
    unsigned error;
    __asm__ __volatile__ ("int $0x40":"=a"(error):"a"(48), "b"(4));
    return error;
}

unsigned kol_area(char *data) {
    unsigned error;
    __asm__ __volatile__ ("int $0x40":"=a"(error):"a"(9), "b"(data), "c"(0xffffffff));
    return error;
}


void kolibri_redraw_window(nsfb_t *nsfb){
	
    kolibri_redraw_window(1);
    kolibri_define_window(100,100,nsfb->width+9,nsfb->height+kol_skin_h(),0x34000080,0x800000FF,"Netsurf for KolibriOS");
    //__menuet__write_text(3,3,0xFFFFFF,"Netsurf",7);
    debug_board_write_str("f65 is mighty with 32 bpp!\n");

    //here put image pixels! it's 32bpp
    f65_32bpp(0,0, nsfb->width, nsfb->height, pixels+1);
    kolibri_redraw_window(2);
}



static bool 
kolibricopy(nsfb_t *nsfb, nsfb_bbox_t *srcbox, nsfb_bbox_t *dstbox) {
   
    char *pixels = nsfb->surface_priv;
    nsfb_bbox_t allbox;
    struct nsfb_cursor_s *cursor = nsfb->cursor;

    nsfb_plot_add_rect(srcbox, dstbox, &allbox);

    int x,y,w,h;
    x = srcbox->x0;
    y = srcbox->y0;
    w = srcbox->x1 - srcbox->x0;
    h = srcbox->y1 - srcbox->y0;
    
    int tx, ty, tw, th;
    
    tx = dstbox->x0;
    ty = dstbox->y0;
    tw = dstbox->x1 - dstbox->x0;
    th = dstbox->y1 - dstbox->y0;
    
    // char pst[255];
    //  sprintf (pst, "Src %d,%d %dx%d Dst %d,%d %dx%d \n", x,y,w,h,tx,ty,tw,th);
    // debug_board_write_str(pst);
    
    int px, py, pp;
    
    for (px=x; px<w; px++) 
	for (py=y;py<h;py++)
	    for (pp=0; pp<4; pp++) {
				
		pixels[4*(px+tx)*nsfb->width+4*(py+ty)+pp]=pixels[4*px*nsfb->width+4*py+pp];
	    }

    kolibri_redraw(nsfb);

    return true;

}

static int kolibri_set_geometry(nsfb_t *nsfb, int width, int height,
				enum nsfb_format_e format) {
    if (nsfb->surface_priv != NULL)
        return -1; /* fail if surface already initialised */

    nsfb->width = width;
    nsfb->height = height;
    nsfb->format = format;

    /* We add one more byte to balance XBGR to BGRX for KolibriOS. */
    /* *4 because we only support 32bpp */

    pixels=(char *)malloc(width*height*4 + 1);
	
    /* select default sw plotters for format */
    if (select_plotters(nsfb) == false)
	return -1; /* Fail if plotter selection failed */
    
    //nsfb->plotter_fns->copy = kolibricopy;

    return 0;
}
unsigned pz, pb;

static int kolibri_initialise(nsfb_t *nsfb) {
    enum nsfb_format_e fmt;

    kol_scancodes(); 

    pz=0;
    pb=0;

    debug_board_write_str("Kolibri Initialise in libnsfb.\n");

    if (nsfb->surface_priv != NULL) {
	debug_board_write_str("Surface already has private surface\n. Abort\n");
        return -1;
    }

    nsfb->surface_priv = pixels;
    nsfb->ptr = pixels;
    nsfb->linelen = (nsfb->width * nsfb->bpp) / 8;
    
    debug_board_write_str("Redraw\n");
    kolibri_redraw(nsfb);

    /*This is for setting flags for mcall40 for events read by a window*/
    kol_set_bitfield_for_wanted_events(EVENT_REDRAW|EVENT_KEY|EVENT_BUTTON|EVENT_MOUSE_CHANGE|(1<<30)|(1<<31)|(1<<7));

    return 0;
}



static int kolibri_finalise(nsfb_t *nsfb) {
    nsfb=nsfb;
    exit(1);
    return 0;
}

int isup(int scan){
    return (scan&0x80)>>7;
}

int scan2key(int scan){
    
    int keycode=(scan&0x0FF7F);
    /* MAIN KB - NUMS */
    if (keycode == 0x02) return NSFB_KEY_1;
    if (keycode == 0x03) return NSFB_KEY_2;
    if (keycode == 0x04) return NSFB_KEY_3;
    if (keycode == 0x05) return NSFB_KEY_4;
    if (keycode == 0x06) return NSFB_KEY_5;
    if (keycode == 0x07) return NSFB_KEY_6;
    if (keycode == 0x08) return NSFB_KEY_7;
    if (keycode == 0x09) return NSFB_KEY_8;
    if (keycode == 0x0A) return NSFB_KEY_9;
    if (keycode == 0x0B) return NSFB_KEY_0;
	
    if (keycode == 0x10) return NSFB_KEY_q;
    if (keycode == 0x11) return NSFB_KEY_w;
    if (keycode == 0x12) return NSFB_KEY_e;
    if (keycode == 0x13) return NSFB_KEY_r;
    if (keycode == 0x14) return NSFB_KEY_t;
    if (keycode == 0x15) return NSFB_KEY_y;
    if (keycode == 0x16) return NSFB_KEY_u;
    if (keycode == 0x17) return NSFB_KEY_i;
    if (keycode == 0x18) return NSFB_KEY_o;
    if (keycode == 0x19) return NSFB_KEY_p;
    if (keycode == 0x1A) return NSFB_KEY_LEFTBRACKET;
    if (keycode == 0x1B) return NSFB_KEY_RIGHTBRACKET;
	
    if (keycode == 0x1E) return NSFB_KEY_a;
    if (keycode == 0x1F) return NSFB_KEY_s;
    if (keycode == 0x20) return NSFB_KEY_d;
    if (keycode == 0x21) return NSFB_KEY_f;
    if (keycode == 0x22) return NSFB_KEY_g;
    if (keycode == 0x23) return NSFB_KEY_h;
    if (keycode == 0x24) return NSFB_KEY_j;
    if (keycode == 0x25) return NSFB_KEY_k;
    if (keycode == 0x26) return NSFB_KEY_l;
	
    if (keycode == 0x2C) return NSFB_KEY_z;
    if (keycode == 0x2D) return NSFB_KEY_x;
    if (keycode == 0x2E) return NSFB_KEY_c;
    if (keycode == 0x2F) return NSFB_KEY_v;
    if (keycode == 0x30) return NSFB_KEY_b;
    if (keycode == 0x31) return NSFB_KEY_n;
    if (keycode == 0x32) return NSFB_KEY_m;
    
    /* TODO: Add a TAB Key here to cycle through fields */
    if (keycode == 0x27) return NSFB_KEY_SEMICOLON;
    if (keycode == 0x28) return NSFB_KEY_QUOTEDBL;
    if (keycode == 0x2B) return NSFB_KEY_BACKSLASH;
    if (keycode == 0x33) return NSFB_KEY_COMMA;
    if (keycode == 0x34) return NSFB_KEY_PERIOD;
    if (keycode == 0x35) return NSFB_KEY_SLASH;
    if (keycode == 0x0C) return NSFB_KEY_MINUS;
    if (keycode == 0x0D) return NSFB_KEY_EQUALS;
	
    if (keycode == 0x0E) return NSFB_KEY_BACKSPACE;
    if (keycode == 0xE053) return NSFB_KEY_DELETE;
    if (keycode == 0x2A) return NSFB_KEY_LSHIFT;
    if (keycode == 0x36) return NSFB_KEY_RSHIFT;
	
    if (keycode == 0x1C) return NSFB_KEY_RETURN;
	
    if (keycode == 0xE04B) return NSFB_KEY_LEFT;
    if (keycode == 0xE04D) return NSFB_KEY_RIGHT;
    if (keycode == 0xE048) return NSFB_KEY_UP;
    if (keycode == 0xE050) return NSFB_KEY_DOWN;
	
    if (keycode == 0x3F) return NSFB_KEY_F5;
	
    if (keycode == 0x39) return NSFB_KEY_SPACE;
    if (keycode == 0x01) return NSFB_KEY_ESCAPE;
	
    if (keycode == 0x38) return NSFB_KEY_LALT;
    if (keycode == 0x1D) return NSFB_KEY_LCTRL;
    if (keycode == 0xE038) return NSFB_KEY_RALT;
    if (keycode == 0xE01D) return NSFB_KEY_RCTRL;
	
	
    if (keycode == 0xE047) return NSFB_KEY_HOME;
    if (keycode == 0xE04F) return NSFB_KEY_END;
    if (keycode == 0xE049) return NSFB_KEY_PAGEUP;
    if (keycode == 0xE051) return NSFB_KEY_PAGEDOWN;
	
    return NSFB_KEY_UNKNOWN;
	
}

int ispowerkey(int scan){
    return (scan&0xE000)>>15;
}


static bool kolibri_input(nsfb_t *nsfb, nsfb_event_t *event, int timeout) {
    int got_event;
    static int scanfull=0;
    char event_num[20];
    
    nsfb = nsfb; /* unused */

    if (timeout >= 0) {
	got_event = kol_wait_for_event_with_timeout(timeout/10);
    } else {
	got_event = kolibri_wait_for_event();
    }   

    if (got_event == 0) {
        return false;
    }

    /* sprintf(event_num, "got_event = %d\n", got_event); */
    /* debug_board_write_str(event_num); */

    event->type = NSFB_EVENT_NONE;

    if (got_event==1) { //redraw event
	kolibri_redraw_window(nsfb);	
    }

    if (got_event==2) { //key pressed
	int scanz = kolibri_get_pressed_key();
    
	//char dbs[64];
    
	//debug_board_write_str("KEY PRESSED\n");
    
	// sprintf (dbs, "FULLKEY BEFORE: F:%x\n", scanfull);
	//debug_board_write_str(dbs);
	
	if (scanz==0xE0) {
	    scanfull=0xE000;
	    return true;
	} else {
	    scanfull=scanfull+scanz;
	}
	
	//sprintf (dbs, "FULLKEY AFTER: F:%x\n", scanfull);
	//debug_board_write_str(dbs);
    
    
	if (isup(scanfull)==1) {
	    event->type = NSFB_EVENT_KEY_UP;} else {
	    event->type = NSFB_EVENT_KEY_DOWN;}
		
	event->value.keycode = scan2key(scanfull);
	
	//sprintf (dbs, "KEY: %x F:%x %d %d\n", scanz, scanfull, isup(scanz), scan2key(scanz));
	//debug_board_write_str(dbs);
	
	scanfull=0;
	
	return true;

    }
	
    if (got_event==3) { //button pressed
	if (kolibri_get_button_id()==1) kolibri_finalise(nsfb);
	return true;
    }

    if (got_event==6) { //mouse event
	unsigned z=kol_mouse_posw();
	unsigned b=kol_mouse_btn();
	int s=kol_mouse_scroll();
	/* char sstr[20]; */
	
	/* sprintf(sstr, "s = %d\n", s); */
	/* debug_board_write_str(sstr); */
	
	if (pz!=z) {
	    event->type = NSFB_EVENT_MOVE_ABSOLUTE;
	    event->value.vector.x = (z&0xffff0000)>>16; //sdlevent.motion.x;
	    event->value.vector.y = z&0xffff; 			//sdlevent.motion.y;
	    event->value.vector.z = 0;
	    pz=z;
	}		
	else if (pb!=b) {	    
	    unsigned diff = pb^b;
	    /* All high bits in the XOR mean that the bit has changed */

	    /* debug_board_write_str("pb!=b"); */
			
	    if(diff&(1<<0)) {			// Left mouse button
		/* debug_board_write_str("KEY_MOUSE_1\n"); */

		event->value.keycode = NSFB_KEY_MOUSE_1;			
		if(b&(1<<0)) {
		    event->type = NSFB_EVENT_KEY_DOWN;
		} else {
		    event->type = NSFB_EVENT_KEY_UP;	   
		}
	    } else if(diff&(1<<1)) {	// Right mouse button		
		/* debug_board_write_str("KEY_MOUSE_3\n"); */
		event->value.keycode = NSFB_KEY_MOUSE_3;
		if(b&(1<<1)) {
		    event->type = NSFB_EVENT_KEY_DOWN;
		} else {
		    event->type = NSFB_EVENT_KEY_UP;   
		}		    		   
	    } else if(diff&(1<<2)) {	// Middle mouse button  
		/* debug_board_write_str("KEY_MOUSE_2\n"); */
		event->value.keycode = NSFB_KEY_MOUSE_2;			
		if(b&(1<<2)) {
		    event->type = NSFB_EVENT_KEY_DOWN;
		} else {
		    event->type = NSFB_EVENT_KEY_UP;		   
		}		    		   
	    } else if(diff&(1<<3)) { 	// 4th mouse button (forward)   
		/* debug_board_write_str("KEY_MOUSE_4\n"); */
		event->value.keycode = NSFB_KEY_MOUSE_4;			
		if(b&(1<<3)) {
		    event->type = NSFB_EVENT_KEY_DOWN;
		} else {
		    event->type = NSFB_EVENT_KEY_UP;		   
		}		    		   
	    } else if(diff&(1<<4)) {		// 5th mouse button (back)  
		/* debug_board_write_str("KEY_MOUSE_5\n"); */
			    
		event->value.keycode = NSFB_KEY_MOUSE_5;			
		if(b&(1<<4)) {
		    event->type = NSFB_EVENT_KEY_DOWN;
		} else {
		    event->type = NSFB_EVENT_KEY_UP;		   
		}	
	    } else { /*The Event 6 did not match any handled cases*/
		char diffstr[40];	    
		sprintf(diffstr, "Unhandled case. pb^b is :  %u", diff);
		debug_board_write_str(diffstr);			    
	    }

	    pb=b;					
	}
	else if(s!=0)
	    {
		short int vert = s&0xffff;
		short int hori = s>>16;

		event->type = NSFB_EVENT_KEY_DOWN;
		/*Handle vertical scroll*/
		if(vert!=0)
		    {
			if(vert>0) /*SCROLL DOWN*/
			    event->value.keycode = NSFB_KEY_MOUSE_5;
			else /*SCROLL UP*/
			    event->value.keycode = NSFB_KEY_MOUSE_4;
		    }
		else /*Since s is not zero and vert is 0. Horizontal scroll*/ 
		    {
			/*Seems like the NS codebase does not have a key dedicated for HSCROLL yet. So use this space when needed*/
		    }
	    }
	return true;
    }
}
/*
		  
  case SDL_MOUSEBUTTONDOWN:
  event->type = NSFB_EVENT_KEY_DOWN;
		  
  switch (sdlevent.button.button) {

  case SDL_BUTTON_LEFT:
  event->value.keycode = NSFB_KEY_MOUSE_1;
  break;

  case SDL_BUTTON_MIDDLE:
  event->value.keycode = NSFB_KEY_MOUSE_2;
  break;

  case SDL_BUTTON_RIGHT:
  event->value.keycode = NSFB_KEY_MOUSE_3;
  break;

  }
  break;

  case SDL_MOUSEBUTTONUP:
  event->type = NSFB_EVENT_KEY_UP;

  switch (sdlevent.button.button) {

  case SDL_BUTTON_LEFT:
  event->value.keycode = NSFB_KEY_MOUSE_1;
  break;

  case SDL_BUTTON_MIDDLE:
  event->value.keycode = NSFB_KEY_MOUSE_2;
  break;

  case SDL_BUTTON_RIGHT:
  event->value.keycode = NSFB_KEY_MOUSE_3;
  break;

  }
  break;

  case SDL_MOUSEMOTION:
  event->type = NSFB_EVENT_MOVE_ABSOLUTE;
  event->value.vector.x = sdlevent.motion.x;
  event->value.vector.y = sdlevent.motion.y;
  event->value.vector.z = 0;
  break;

  case SDL_QUIT:
  event->type = NSFB_EVENT_CONTROL;
  event->value.controlcode = NSFB_CONTROL_QUIT;
  break;

  case SDL_USEREVENT:
  event->type = NSFB_EVENT_CONTROL;
  event->value.controlcode = NSFB_CONTROL_TIMEOUT;
  break;

  }
*/


static int kolibri_claim(nsfb_t *nsfb, nsfb_bbox_t *box) {
    /*
      if ((cursor != NULL) &&
      (cursor->plotted == true) &&
      (nsfb_plot_bbox_intersect(box, &cursor->loc))) {
      nsfb_cursor_clear(nsfb, cursor);
      } */
    return 0; //stub yet
}

static int kolibri_cursor(nsfb_t *nsfb, struct nsfb_cursor_s *cursor) {
    return true; //stub yet
}



static int kolibri_update(nsfb_t *nsfb, nsfb_bbox_t *box) {
    /* Do redraw window here! */
    kolibri_redraw(nsfb);
    return 0;
}

const nsfb_surface_rtns_t kolibri_rtns = {
    .initialise = kolibri_initialise,
    .finalise = kolibri_finalise,
    .input = kolibri_input,
    .claim = kolibri_claim,
    .update = kolibri_update,
    .cursor = kolibri_cursor,
    .geometry = kolibri_set_geometry,
};

NSFB_SURFACE_DEF(kolibri, NSFB_SURFACE_KOLIBRI, &kolibri_rtns)
