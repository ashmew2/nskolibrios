#include "kolibri_http.h"

int kolibri_http_init(void)
{
  int asm_init_status = init_network_asm();

  if(asm_init_status == 0)
    return 0;
  else
    return 1;
}

void http_send(struct http_msg *handle, char *data, int remaining_length) {

  int sent = 0;
  int total_sent = 0;
  int retries_left = 5;

  while(remaining_length > 0) {
    sent = http_send_asm(handle, data + total_sent, remaining_length);

    if(sent == -1) {
      --retries_left;

      if(retries_left)
	continue;
      else {
	debug_board_write_str("ERROR: HTTP LIBRARY : Could not send data.\n");
	__asm__ __volatile__("int3");
      }
    }

    remaining_length -= sent;
    total_sent += sent;
  }
}
