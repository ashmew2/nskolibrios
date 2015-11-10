/* ------------------------------ */
/* Part about GUI LLCACHE TABLE. Will contain all functions required as well. */
/* ------------------------------ */
nserror kolibri_initialise(const struct llcache_store_parameters *parameters)
{

}


nserror kolibri_finalise(void)
{

}


nserror kolibri_store(struct nsurl *url, enum backing_store_flags flags,
		 uint8_t *data, const size_t datalen)
{

}


nserror kolibri_fetch(struct nsurl *url, enum backing_store_flags flags,
		 uint8_t **data, size_t *datalen)
{

}


nserror kolibri_release(struct nsurl *url, enum backing_store_flags flags)
{

}


nserror kolibri_invalidate(struct nsurl *url)
{

}



struct gui_llcache_table kolibri_gui_llcache_table = {
	.initialise = kolibri_initialise,
	.finalise = kolibri_finalise,
	.store = kolibri_store,
	.fetch = kolibri_fetch,
	.release = kolibri_release,
	.invalidate = kolibri_invalidate
};
