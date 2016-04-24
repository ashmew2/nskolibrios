#include "kolibri_http.h"

int kolibri_http_init(void)
{
  int asm_init_status = init_network_asm();

  if(asm_init_status == 0)
    return 0;
  else
    return 1;
}
