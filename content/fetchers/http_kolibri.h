#ifndef HTTP_KOLIBRI_H
#define HTTP_KOLIBRI_H

/*We don't have a sys/select.h header */
/*#include <sys/select.h>*/

#include <time.h>

#include "utils/errors.h"
#include "content/fetchers.h"

/* http_msg defined in kolibrios/kolibri_http.h */
extern struct http_msg;

bool http_kolibri_handle_init(void);

/* TODO: multi handle should be a linked list of http_msg just like we implemented in Sourcerer's port */
struct http_kolibri_multi_handle {

};

extern struct http_kolibri_multi_handle* fetch_http_kolibri_multi;

int http_kolibri_multi_fdset(struct http_kolibri_multi_handle *multi_handle,
			     fd_set *read_fd_set,
			     fd_set *write_fd_set,
			     fd_set *exc_fd_set,
			     int *max_fd);

bool fetch_http_kolibri_initialise(lwc_string *scheme);
bool fetch_http_kolibri_can_fetch(const struct nsurl *url);

/* TODO: This should be simplified, CURL makes it overly complex */
void * fetch_http_kolibri_setup(struct fetch *parent_fetch, struct nsurl *url,
		bool only_2xx, bool downgrade_tls, const char *post_urlenc,
		const struct fetch_multipart_data *post_multipart,
		const char **headers);
bool fetch_http_kolibri_start(void *);
void fetch_http_kolibri_abort(void *);
void fetch_http_kolibri_free(void *);
void fetch_http_kolibri_poll(lwc_string *scheme);
void fetch_http_kolibri_finalise(lwc_string *scheme);


nserror fetch_http_kolibri_register(void);

time_t kolibri_getdate(char *datestring);
#endif /* HTTP_KOLIBRI_H */
