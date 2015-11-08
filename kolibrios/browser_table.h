/* ------------------------------ */
/* Part about Browser table. Will contain all functions required as well. */
/* ------------------------------ */
nserror kolibri_schedule_cb(int t, void (*callback)(void *p), void *p);

static struct gui_browser_table kolibri_browser_table = {
    .schedule = kolibri_schedule_cb,
    };

nserror kolibri_schedule_cb(int t, void (*callback)(void *p), void *p)
{
  return NSERROR_OK;
}
