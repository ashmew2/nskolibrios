#ifndef KOLIBRI_DEBUG_H
#define KOLIBRI_DEBUG_H

/* Write a printf() like function (variable argument list) for 
   writing to debug board */

void debug_board_write_byte(const char ch);
void debug_board_write_str(const char* str);
void debug_board_printf(const char *format, ...);

#endif /* KOLIBRI_DEBUG_H */
