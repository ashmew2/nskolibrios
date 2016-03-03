#include "http_kolibri.h"
#include "nsurl.h"

struct http_kolibri_multi_handle* fetch_http_kolibri_multi;

/* In case we need to do something fancy for setting up the htp handle */
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
  debug_board_write_str("Initialize HTTP fetch for: ");
  debug_board_write_str(lwc_string_data(scheme));
  debug_board_write_str("\n");
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
  
  
}

bool fetch_http_kolibri_start(void *fetch) {
  struct http_msg * http_handle = http_get_asm((nsurl_get_component((struct fetch *)fetch -> url), 
						NSURL_PATH), 0, 0, 0);
  
  (struct fetch *)fetch -> fetcher_handle = http_msg;

  /* Add this fetcher to a global list so that we can poll on it later */
}

void fetch_http_kolibri_abort(void *fetch) {
  
}

void fetch_http_kolibri_free(void *fetch) {
  http_free_asm((struct http_msg *)(fetch->fetcher_handle));
  /* TODO: Do we need to free the fetcher here as well? */
}

void fetch_http_kolibri_poll(lwc_string *scheme) {
  /* Need a global linked list of all http fetchers here */
  /* All those http fetchers are called with http_receive_asm here */
  
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

	return fetcher_add(lwc_string_ref(corestring_lwc_https), kolibri_http_ops);

	/* TODO: Since KolibriOS does not have an HTTPS implementation yet,
	   add it later using a similar approach to HTTP. */

	return NSERROR_OK;
}

time_t kolibri_getdate(char *datestring)
{
}
