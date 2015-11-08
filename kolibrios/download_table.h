/* ------------------------------ */
/* Part about GUI DOWNLOAD table. Will contain all functions required as well. */
/* ------------------------------ */

struct gui_download_window *kolibri_create_download_window(struct download_context *ctx, struct gui_window *parent);
nserror kolibri_download_data(struct gui_download_window *dw,	const char *data, unsigned int size);
void kolibri_download_error(struct gui_download_window *dw, const char *error_msg);
void kolibri_download_done(struct gui_download_window *dw);

static struct gui_download_table kolibri_download_table {
    .create = kolibri_create_download_window,
	.data = kolibri_download_data,
	.error = kolibri_download_error,
	.done = kolibri_download_done;
    };

