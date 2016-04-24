/* ------------------------------ */
/* Part about GUI CLIPBOARD table. Will contain all functions required as well. */
/* ------------------------------ */

void kolibri_get_clipboard_contents(char **buffer, size_t *length)
{

}


void kolibri_set_clipboard_contents(const char *buffer, size_t length, nsclipboard_styles styles[], int n_styles)
{

}

static struct gui_clipboard_table kolibri_clipboard_table = {
  .get = kolibri_get_clipboard_contents,
    .set = kolibri_set_clipboard_contents,
    };

