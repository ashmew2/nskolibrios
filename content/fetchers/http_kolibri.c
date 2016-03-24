#include "utils/nsurl.h"
#include "utils/corestrings.h"
#include "http_kolibri.h"
#include "kolibrios/kolibri_http.h"

#include "content/fetch.h"

/* TODO: Make these symbols static as they don't need to be exported. Only the register function needs to be non static */

struct kolibri_fetch {
  struct fetch *fetch_handle;
  struct http_msg *http_handle;
  struct nsurl *url;
  struct kolibri_fetch *next_kolibri_fetch;
};


struct kolibri_fetch *fetcher_head;
/* http_handle for this dummy pointer is always NULL */

struct kolibri_fetch *fetcher_tail;

struct http_kolibri_multi_handle* fetch_http_kolibri_multi;

/* In case we need to do something fancy for setting up the http handle */
bool http_kolibri_handle_init(void)
{
  /* Do stuff about setting up the HTTP handle here */
  /* Since http.obj just allocates a handle with http_get, I think we can rely on the lib to provide a handle */
  /* The problem is that we need to return a new handle here.. or maybe not. */

  return true;
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
  fetcher_tail = fetcher_head;
  fetcher_head -> http_handle = NULL;
  fetcher_head -> next_kolibri_fetch = NULL;

  debug_board_write_str("Initialed HTTP fetcher for Netsurf on Kolibri.\n");
  return true;
}

bool fetch_http_kolibri_can_fetch(const struct nsurl *url) {

  /* This simply means that we want to make sure NSURL has a HOST component */
  /* Maybe we should change this to support only HTTP hosts, or maybe not yet ;P */
  return nsurl_has_component(url, NSURL_HOST);
}

void * fetch_http_kolibri_setup(struct fetch *parent_fetch, struct nsurl *url,
		bool only_2xx, bool downgrade_tls, const char *post_urlenc,
		const struct fetch_multipart_data *post_multipart,
		const char **headers) {

  struct kolibri_fetch *new_kfetcher = malloc(sizeof(struct kolibri_fetch));

  new_kfetcher -> next_kolibri_fetch = NULL;
  new_kfetcher -> fetch_handle = parent_fetch;
  
  /* We will initilize the HTTP handle in _star() function later when required */
  new_kfetcher -> http_handle = NULL;

  /* Set the URL for the new fetch */
  new_kfetcher -> url = url;

  /* Add this fetcher to a global list so that we can poll on it later */
  fetcher_tail->next_kolibri_fetch = new_kfetcher;
  fetcher_tail = new_kfetcher;

  return new_kfetcher;
}

bool fetch_http_kolibri_start(void *kfetch) {
  debug_board_write_str("Starting fetch_http_kolibrI_start\n");
  struct kolibri_fetch *fetch = (struct kolibri_fetch *)kfetch;

  fetch->next_kolibri_fetch = NULL;
  __asm__ __volatile__("int3");
  //  fetch->http_handle = 
  http_get_asm("www.kolibrios.org", 0, 0, 0);
  //nsurl_get_component(fetch->url, NSURL_PATH)
  debug_board_write_str("http_get_asm returned.\n");

  return NSERROR_OK;
}

void fetch_http_kolibri_abort(void *fetch) {
  
}

void fetch_http_kolibri_free(void *kfetch) {
  struct kolibri_fetch *fetch = (struct kolibri_fetch *)kfetch;
  http_free_asm(fetch -> http_handle);

  /* TODO: Remove this fetcher handle from the linked list of kolib 
     fetchers as well..or in finalize? */
  /* TODO: Do we need to free the fetcher here as well? */
}

void fetch_http_kolibri_poll(lwc_string *scheme) {
  /* Need a global linked list of all http fetchers here */
  /* All those http fetchers are called with http_receive_asm here */

  /* Assume all schemes are HTTP for now */
  struct kolibri_fetch *poller = fetcher_head -> next_kolibri_fetch;
  
  while(fetcher_head!=NULL)
    {
      http_receive_asm(poller->http_handle);
      poller = poller -> next_kolibri_fetch;
    }
}

void fetch_http_kolibri_finalise(lwc_string *scheme) {

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
