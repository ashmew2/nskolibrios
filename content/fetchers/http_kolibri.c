#include "utils/nsurl.h"
#include "utils/corestrings.h"
#include "http_kolibri.h"
#include "kolibrios/kolibri_http.h"

#include "content/fetch.h"

/* TODO: Make these symbols static as they don't need to be exported. Only the register function needs to be non static */

struct kolibri_fetch {
  struct fetch *fetch_handle;
  struct http_msg *http_handle;
  struct kolibri_fetch *next_kolibri_fetch;
  char *url_string;
  size_t url_length;
  bool finished;
  bool data_processed;
  bool receive_complete;
  bool headers_processed;
};

struct kolibri_fetch *fetcher_head;
/* http_handle for this dummy pointer is always NULL */

struct http_kolibri_multi_handle* fetch_http_kolibri_multi;

/* In case we need to do something fancy for setting up the http handle */
bool http_kolibri_handle_init(void)
{
  /* Do stuff about setting up the HTTP handle here */
  /* Since http.obj just allocates a handle with http_get, I think we can rely on the lib to provide a handle */
  /* The problem is that we need to return a new handle here.. or maybe not. */

  return true;
}


void print_linked_list(void) {
  debug_board_write_str("Start of LL: \n");
  struct kolibri_fetch *f = fetcher_head;
  char addp[40];

  while(f!=NULL) {
    sprintf(addp, "%p\n", (void *)f);
    debug_board_write_str(addp);
    f = f->next_kolibri_fetch;
  }
  
  debug_board_write_str("end of LL: \n====\n");
}


int http_kolibri_multi_fdset(struct http_kolibri_multi_handle *multi_handle,
                           fd_set *read_fd_set,
                           fd_set *write_fd_set,
                           fd_set *exc_fd_set, int *max_fd)
{

}


bool fetch_http_kolibri_initialise(lwc_string *scheme) {
  debug_board_write_str("Initializing HTTP fetcher for Netsurf on Kolibri.\n");

  /* Should these be in register() instead? . Depends on if register is called once or this is called once */

  fetcher_head = malloc(sizeof(struct kolibri_fetch));
  fetcher_head -> http_handle = NULL;
  fetcher_head -> next_kolibri_fetch = NULL;

  char addr[30];
  sprintf(addr, "[init]fetcher_head: %p\n", (void *)fetcher_head);
  debug_board_write_str(addr);


  debug_board_write_str("Initialed HTTP fetcher for Netsurf on Kolibri.\n");
  return true;
}

bool fetch_http_kolibri_can_fetch(const struct nsurl *url) {

  /* This simply means that we want to make sure NSURL has a HOST component */
  /*FIXME  : ADd a check for HTTP here as we dont ahve HTTPS right now or other protocols. */

  return true;
}

void * fetch_http_kolibri_setup(struct fetch *parent_fetch, struct nsurl *url,
		bool only_2xx, bool downgrade_tls, const char *post_urlenc,
		const struct fetch_multipart_data *post_multipart,
		const char **headers) {

  struct kolibri_fetch *new_kfetcher = malloc(sizeof(struct kolibri_fetch));
  char addr[40];

  sprintf(addr, "[Init Setup kf]Addr: %p\n", (void *)new_kfetcher);
  debug_board_write_str(addr);
  //  __asm__ __volatile__("int3");
  
  new_kfetcher -> next_kolibri_fetch = NULL;
  new_kfetcher -> fetch_handle = parent_fetch;
  new_kfetcher -> http_handle = NULL;
  new_kfetcher -> finished = false;
  new_kfetcher -> data_processed = false;
  new_kfetcher -> receive_complete = false;
  new_kfetcher -> headers_processed = false;
  new_kfetcher -> url_string = NULL;

  /* We will initilize the HTTP handle in _start() function later when required */  

  /* Set the URL for the new kolibri fetch*/
  char *url_safety = new_kfetcher->url_string;
  size_t url_len;
  if(nsurl_get_utf8(url, &url_safety, &url_len) != NSERROR_OK)
    {
      debug_board_write_str("nsurl_get_utf8 failed!");
      exit(3);
    }

  sprintf(addr, "[after-utf8]Address: %p\n", (void *)new_kfetcher);
  debug_board_write_str(addr);

  new_kfetcher->url_length = url_len;
  sprintf(addr, "[SetupAfterLENAssign]Add: %p\n", (void *)new_kfetcher);
  debug_board_write_str(addr);
  
  new_kfetcher->url_string = url_safety;

  sprintf(addr, "[SetupAfterURLAssign]Add: %p\n", (void *)new_kfetcher);
  debug_board_write_str(addr);
  
  /* sprintf(addr, "Setup for URL : %s\n", new_kfetcher->url_string);  /\* 76c0 bp *\/ */
  /* sprintf(addr, "Setup for URL : %s\n", url_safety); */
  /* debug_board_write_str(addr); */

  //  __asm__ __volatile__("int3");
  sprintf(addr, "[Setup hh]Address: %p\n", (void *)(new_kfetcher->http_handle));
  debug_board_write_str(addr);

  struct kolibri_fetch *f = fetcher_head;

  /* Go to the end of the LL */
  while(f->next_kolibri_fetch!=NULL)
    f = f->next_kolibri_fetch;

  f->next_kolibri_fetch = new_kfetcher;

  return new_kfetcher;
}

bool fetch_http_kolibri_start(void *kfetch) {

  debug_board_write_str("Starting fetch_http_kolibri_start\n");
  struct kolibri_fetch *fetch = (struct kolibri_fetch *)kfetch;

  //  __asm__ __volatile__("int3");
  /* FIXME: add true or false return depending upon http_get_asm return value */
  
  fetch->http_handle = http_get_asm(fetch->url_string, 0, 0, 0);

  if(fetch -> http_handle == NULL) {
    debug_board_write_str("http_handle is NULL when starting. Aborting.\n");
    exit(2);
  }
  else
    {
      char addr[40];
      sprintf(addr, "Address: %p\n", (void*)(fetch->http_handle));
      debug_board_write_str(addr);
      sprintf(addr, "cptr: %p\n", (void*)(fetch->http_handle->content_ptr));
      debug_board_write_str(addr);
      sprintf(addr, "header: %x\n", (fetch->http_handle->http_header));
      debug_board_write_str(addr);
    }

  //nsurl_get_component(fetch->url, NSURL_PATH)
  debug_board_write_str("Started fetch using http_get_asm.\n");
  return true;
  //return NSERROR_OK;
}

void fetch_http_kolibri_abort(void *fetch) {
  
}

void fetch_http_kolibri_free(void *kfetch) {
  debug_board_write_str("Inside fetch_http_kolibri_free!!!!\n");
  //  __asm__ __volatile__("int3");

  struct kolibri_fetch *kfetch_to_free = (struct kolibri_fetch *)kfetch;
  struct kolibri_fetch *delete_ptr = fetcher_head;

  debug_board_write_str("Calling http_free_asm\n");

  http_free_asm(kfetch_to_free -> http_handle);

  debug_board_write_str("http_free_asm did not crash\n");
  /* Remove this fetcher handle from the linked list */
  
  while(delete_ptr!=NULL) {
    if(delete_ptr->next_kolibri_fetch == kfetch_to_free)
      {
	char addr[30];
	sprintf(addr, "[free]kfetcher: %p\n", (void *)fetcher_head);
	debug_board_write_str(addr);

	delete_ptr->next_kolibri_fetch = kfetch_to_free->next_kolibri_fetch;
	debug_board_write_str("kfetch removed from Linked List\n");

	//	__asm__ __volatile__("int3");
	if(delete_ptr->next_kolibri_fetch == NULL) {
	  debug_board_write_str("next is now NULL!"); 
	  if(fetcher_head->next_kolibri_fetch == NULL)
	    debug_board_write_str("FETCHER HEAD NEXT IS NULL!\n");
	  else
	    debug_board_write_str("NOOOOOOOOOOOOOOOOOOOOOOOOOOOOT NULL FETCHER HEAD\n");
	}
	
	else
	  debug_board_write_str("next is not NULL!");
				
	break;
      }
    delete_ptr=delete_ptr->next_kolibri_fetch;
  }

  free(kfetch_to_free);

  print_linked_list();
  //  __asm__ __volatile__("int3");
  debug_board_write_str("Leaving kolibri_fetch_free\n");
}

void fetch_http_kolibri_poll(lwc_string *scheme) {

  /* Need a global linked list of all http fetchers here */
  /* All those http fetchers are called with http_receive_asm here */
  //  debug_board_write_str("Inside fetch_http_kolibri_poll\n");
  /* Assume all schemes are HTTP for now */
  struct kolibri_fetch *poller = fetcher_head -> next_kolibri_fetch;
  if(fetcher_head -> next_kolibri_fetch == NULL)
    debug_board_write_str("Poller: NEXT IS NULL!\n");
  else
    debug_board_write_str("Poller: NEXT IS NOT NULL!\n");

  //  __asm__ __volatile__("int3");
  while(poller!=NULL)
    {
      if(poller->receive_complete == false) {

	int x = http_receive_asm(poller->http_handle);
	if(x == 0)
	  poller->receive_complete = true;
	else {
	  poller = poller->next_kolibri_fetch;
	  continue;
	}
      }

      if(poller -> finished == false && poller->receive_complete == true) {
	fetch_msg msg;

	/* char data[30]; */
	/* snprintf(data, 29, "%s", poller-> http_handle -> content_ptr); */
	/* debug_board_write_str(data); */
	
	/* if(poller->http_handle->status == 301 || poller->http_handle->status == 302) { */
	/*   debug_board_write_str("Received 301 or 302, doing FETCH_REDIRECT callback"); */
	/*   msg.type = FETCH_REDIRECT; */
	/*   fetch_send_callback(&msg, poller->fetch_handle); */
	/* } */
	/* else */ 

	if(poller->http_handle->status == 200) {

	  debug_board_write_str("200 OK! HTTP Code.\n");

	  //	  __asm__ __volatile__("int3");

	  if(poller->headers_processed == false) {
	    int i, newline;
	    size_t header_length;
	    char *header;

	    debug_board_write_str("Do FETCH_HEADER\n");
	    msg.type = FETCH_HEADER;

	    header = (const uint8_t *)(&(poller -> http_handle -> http_header));
	    header_length  = (size_t) poller -> http_handle -> header_length;

	    /* msg.data.header_or_data.len = (size_t) poller -> http_handle -> header_length; */
	    /* msg.data.header_or_data.buf = (const uint8_t *)(&(poller -> http_handle -> http_header)); */

	    for(i = 0, newline = 0;i < header_length; i++)
	      {
		if(header[i] == '\n')
		  {
		    char data[80];
		    msg.data.header_or_data.len = i - newline;
		    msg.data.header_or_data.buf = (const uint8_t *) (header + newline);
		  
		    snprintf(data, msg.data.header_or_data.len, "%u : %s\n", msg.data.header_or_data.len, msg.data.header_or_data.buf);
		    debug_board_write_str("feeding:");
		    debug_board_write_str(data);
		    debug_board_write_str("\n");
		    
		    /* LOG(("buf inside send_header_cb is : %.*s\n", i - newline, header+newline)); */

		    newline = i+1;
		    fetch_send_callback(&msg, poller->fetch_handle);
		  }
	      }
	    poller->headers_processed = true;
	  }

	  /* Only do FETCH_DATA if header processing is complete, otherwise dont */
	  if(poller->headers_processed == true) {
	    debug_board_write_str("Do FETCH_DATA\n");
	    //	    __asm__ __volatile__("int3");

	    msg.type = FETCH_DATA;
	    msg.data.header_or_data.buf = (const uint8_t *) poller -> http_handle -> content_ptr;
	    msg.data.header_or_data.len = (size_t) poller -> http_handle -> content_length;
	    fetch_send_callback(&msg, poller->fetch_handle);
	  }
	  poller->data_processed = true;
	}
	else {
	  debug_board_write_str("Error! Unhandled HTTP REturn Code.\n");
	  exit(1);
	}
	
	if(poller->data_processed == true) {
	  debug_board_write_str("Do FETCH_FINISHED\n");
	  msg.type = FETCH_FINISHED;
	  fetch_send_callback(&msg, poller->fetch_handle);
	  poller->finished = true;
	}
      }
      else if(poller->finished == true) {
	/* time to free the fetcher  */
	char addr[30];
	print_linked_list();
	debug_board_write_str("Calling fetch_free..\n");

	sprintf(addr, "[callfree]poller: %p\n", (void *)poller);
	debug_board_write_str(addr);

	fetch_free(poller->fetch_handle);
      }
      poller = poller -> next_kolibri_fetch;
    }
}


void fetch_http_kolibri_finalise(lwc_string *scheme) {
  debug_board_write_str("Finalize fetch called!\n");
}

nserror fetch_http_kolibri_register(void)
{
	const struct fetcher_operation_table kolibri_http_ops = {
		.initialise = fetch_http_kolibri_initialise,
		.acceptable = fetch_http_kolibri_can_fetch,
		.setup = fetch_http_kolibri_setup,
		.start = fetch_http_kolibri_start,
		.abort = fetch_http_kolibri_abort,
		.free = fetch_http_kolibri_free,
		.poll = fetch_http_kolibri_poll,
		.finalise = fetch_http_kolibri_finalise
	};

	/* Register the fetcher operation table above for HTTP */

	return fetcher_add(lwc_string_ref(corestring_lwc_http), &kolibri_http_ops);

	/* TODO: Since KolibriOS does not have an HTTPS implementation yet,
	   add it later using a similar approach to HTTP. */

	return NSERROR_OK;
}

time_t kolibri_getdate(char *datestring)
{

}
