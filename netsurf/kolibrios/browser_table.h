/* ------------------------------ */
/* Part about Browser table. Will contain all functions required as well. */
/* ------------------------------ */
nserror kolibri_schedule_cb(int t, void (*callback)(void *p), void *p);
void kolibri_browser_quit(void);

struct kolibri_callback {
  void (*cb)(void *p);
  void *arg;
  struct kolibri_callback *next_cb;
} head_dummy;

struct kolibri_callback *kcb_head = &head_dummy;
struct kolibri_callback *kcb_tail = &head_dummy;

static struct gui_browser_table kolibri_browser_table = {
    .schedule = kolibri_schedule_cb,
    .quit = kolibri_browser_quit,
    };

nserror kolibri_schedule_cb(int t, void (*callback)(void *p), void *p)
{
  /* debug_board_write_str("kolibri_schedule_cb called!\n"); */
  struct kolibri_callback *new_cb = malloc(sizeof(struct kolibri_callback));
  new_cb->arg = p;
  new_cb->cb = callback;
  new_cb->next_cb = NULL;

  kcb_tail -> next_cb = new_cb;
  kcb_tail = new_cb;

  return NSERROR_OK;
}

bool kolibri_schedule_run(void)
{
  if(kcb_head != kcb_tail)
    return false;

  struct kolibri_callback *runner = kcb_head -> next_cb;

  while(runner != NULL)
    {
      runner->cb(runner->arg);
      runner = runner -> next_cb;
    }

  return true;
  /* TODO: Free the whole list of callbacks */
}

void kolibri_browser_quit(void)
{
  debug_board_write_str("Netsurf is shutting down Normally..GoodBye.");
  /* TODO: Do the actual killing of process with mcall -1 */
  /* Maybe set Kolibri_quit = 1 here */
}
