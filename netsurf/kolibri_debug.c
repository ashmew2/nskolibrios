/* Write a printf() like function (variable argument list) for 
   writing to debug board */

void debug_board_write_byte(const char ch){
    __asm__ __volatile__(
    "int $0x40"
    :
    :"a"(63), "b"(1), "c"(ch));
}

void debug_board_write_str(const char* str){
  while(*str)
    debug_board_write_byte(*str++);
}
