/* Instructions to compile this file with newlib (Assuming you have set up environment

kos32-gcc -c -I/home/bob/kolibrios/contrib/sdk/sources/newlib/libc/include -I/home/bob/kolibrios/contrib/sdk/sources/libpng/ -I/home/bob/kolibrios/contrib/sdk/sources/zlib -I/home/bob/kolibrios/contrib/sdk/sources/freetype/include -I/home/bob/kolibrios/contrib/sdk/sources/freetype/include -I/home/bob/kolibrios/contrib/sdk/sources/SDL-1.2.2/include/ -std=c99 -D_KOLIBRIOS -Dnskolibrios -g   -Wundef   -U_Win32 -U_WIN32 -U__MINGW32__ SDLTest.c

  kos32-ld SDLTest.o -T/home/bob/kolibrios/contrib/sdk/sources/newlib/libc/app.lds -nostdlib -static --image-base 0 -lgcc -L/home/autobuild/tools/win32/mingw32/lib /home/autobuild/tools/win32/lib/libdll.a /home/autobuild/tools/win32/lib/libapp.a /home/autobuild/tools/win32/lib/libSDL.a /home/autobuild/tools/win32/lib/libc.dll.a -static -o sdltest

objcopy -O binary sdltest

Now sdltest is your binary to run on Kolibri for SDL Demo.

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

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

int main(int argc, char* argv[])
{
  struct stat statstr;
  //  __asm__ __volatile__("int3");
  int status = stat("/usbhd0/1/tex.txt", &statstr);
  if (status == 0)
    debug_board_write_str("stat worked with 0 return!");
  else
    debug_board_write_str("stat failed with non zero!");  
  
  return 0;
}
