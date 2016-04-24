/* ------------------------------ */
/* Part about GUI UTF8 TABLE. Will contain all functions required as well. */
/* ------------------------------ */

nserror kolibri_utf8_to_local(const char *string, size_t len, char **result)
{

}

nserror kolibri_local_to_utf8(const char *string, size_t len, char **result)
{

}

struct gui_utf8_table kolibri_gui_utf8_table = {
  .utf8_to_local = kolibri_utf8_to_local,
  .local_to_utf8 = kolibri_local_to_utf8
};
