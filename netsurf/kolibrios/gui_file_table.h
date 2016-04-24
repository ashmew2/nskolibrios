/* ------------------------------ */
/* Part about GUI FILE TABLE. Will contain all functions required as well. */
/* ------------------------------ */
nserror kolibri_mkpath(char **str, size_t *size, size_t nemb, va_list ap)
{

}

nserror kolibri_basename(const char *path, char **str, size_t *size)
{

}

nserror kolibri_nsurl_to_path(struct nsurl *url, char **path)
{

}

nserror kolibri_path_to_nsurl(const char *path, struct nsurl **url)
{

}

nserror kolibri_mkdir_all(const char *fname)
{

}


struct gui_file_table kolibri_gui_file_table = {
	/* Mandantory entries */
  .mkpath = kolibri_mkpath,
  .basename = kolibri_basename,
  .nsurl_to_path = kolibri_nsurl_to_path,
  .path_to_nsurl = kolibri_path_to_nsurl,
  .mkdir_all = kolibri_mkdir_all
};
