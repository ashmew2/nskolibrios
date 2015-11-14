#include "http_kolibri.h"

struct http_kolibri_multi_handle* fetch_http_kolibri_multi;

struct http_kolibri_handle* http_kolibri_handle_init(void)
{

}

void http_kolibri_handle_cleanup(struct http_kolibri_handle *handle)
{
  
}

void http_kolibri_handle_free(struct http_kolibri_handle *handle)
{

}

int http_kolibri_multi_fdset(struct http_kolibri_multi_handle *multi_handle,
                           fd_set *read_fd_set,
                           fd_set *write_fd_set,
                           fd_set *exc_fd_set, int *max_fd)
{

}


bool fetch_http_kolibri_initialise(lwc_string *scheme) {

}

bool fetch_http_kolibri_can_fetch(const struct nsurl *url) {

}

void * fetch_http_kolibri_setup(struct fetch *parent_fetch, struct nsurl *url,
		bool only_2xx, bool downgrade_tls, const char *post_urlenc,
		const struct fetch_multipart_data *post_multipart,
		const char **headers) {

}

bool fetch_http_kolibri_start(void *fetch) {

}

void fetch_http_kolibri_abort(void *fetch) {

}

void fetch_http_kolibri_free(void *fetch) {

}

void fetch_http_kolibri_poll(lwc_string *scheme) {

}

void fetch_http_kolibri_finalise(lwc_string *scheme) {

}

nserror fetch_http_kolibri_register(void)
{
	const struct fetcher_operation_table fetcher_ops = {
		.initialise = fetch_http_kolibri_initialise,
		.acceptable = fetch_http_kolibri_can_fetch,
		.setup = fetch_http_kolibri_setup,
		.start = fetch_http_kolibri_start,
		.abort = fetch_http_kolibri_abort,
		.free = fetch_http_kolibri_free,
		.poll = fetch_http_kolibri_poll,
		.finalise = fetch_http_kolibri_finalise
	};

	return NSERROR_OK;
}

time_t kolibri_getdate(char *datestring)
{
}
