/*The libiconv.a file that we have in built-libs defines these functions as libiconv_*()
Maybe best is to call the libiconv() functions from here rather than hacking NS sources.
As this is a KolibriOS dependent issue.
*/
#include<iconv.h>
int iconv_close (iconv_t __cd)
{

}



