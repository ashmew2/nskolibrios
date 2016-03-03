/* ------------------------------ */
/* Part about Browser table. Will contain all functions required as well. */
/* ------------------------------ */
nserror kolibri_schedule_cb(int t, void (*callback)(void *p), void *p);
void kolibri_browser_quit(void);

static struct gui_browser_table kolibri_browser_table = {
    .schedule = kolibri_schedule_cb,
    .quit = kolibri_browser_quit,
    };

nserror kolibri_schedule_cb(int t, void (*callback)(void *p), void *p)
{
  return NSERROR_OK;
}

void kolibri_browser_quit(void)
{
  debug_board_write_str("Netsurf is shutting down Normally..GoodBye.");
}
