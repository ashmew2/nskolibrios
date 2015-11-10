/* ------------------------------ */
/* Part about GUI FETCHER table. Will contain all functions required as well. */
/* ------------------------------ */

const char *kolibri_get_mimetype_file(const char *unix_path)
{

}


struct nsurl* kolibri_get_resource_url(const char *path)
{

}

 /* Can be used to always load some .html file probably */

nserror kolibri_get_resource_data(const char *path, const uint8_t **data, size_t *data_len)
{

}


nserror kolibri_release_resource_data(const uint8_t *data)
{

}

 /*get_ and release_ functions work in unison one after another*/

char *kolibri_find_mimetype_file(const char *ro_path)
{

}


struct gui_fetch_table kolibri_fetch_table = {
  .filetype = kolibri_get_mimetype_file;
  .get_resource_url = kolibri_get_resource_url;
  .get_resource_data = kolibri_get_resource_data;
  .release_resource_data = kolibri_release_resource_data;
  .mimetype = kolibri_find_mimetype_file;
};

struct gui_fetch_table kolibri_fetch_table = {
    .filetype = kolibri_get_mimetype_file,
    .get_resource_url = kolibri_get_resource_url,
    .get_resource_data = kolibri_get_resource_data,
    .release_resource_data = kolibri_release_resource_data,
    .mimetype = kolibri_find_mimetype_file,
    };
