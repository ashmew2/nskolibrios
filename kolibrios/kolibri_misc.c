#include "desktop/font.h"
#include <time.h>

void warn_user(const char *warning, const char *detail)
{

}

static bool kolibrios_font_position_in_string(const plot_font_style_t * fstyle,const char *string,
						size_t length,int x, size_t *char_offset, int *actual_x )
{
}

static bool kolibrios_font_split( const plot_font_style_t * fstyle, const char *string,
					  size_t length,int x, size_t *char_offset, int *actual_x )
{

}

static bool kolibrios_font_width( const plot_font_style_t *fstyle, const char * str,
					  size_t length, int * width  )
{

}

const struct font_functions nsfont = {
	kolibrios_font_width,
	kolibrios_font_position_in_string,
	kolibrios_font_split
};

int inet_aton(const char *cp, struct in_addr *inp) {

}

char *strptime(const char *s, const char *format, struct tm *tm) {

}

int atexit(void (*function)(void)) {

}
