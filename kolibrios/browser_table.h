/* ------------------------------ */
/* Part about Browser table. Will contain all functions required as well. */
/* ------------------------------ */
nserror kolibri_schedule_cb(int t, void (*callback)(void *p), void *p);
void kolibri_browser_quit(void);

struct kolibri_callback {
  void (*callback)(void *p) cb;
  void *arg;
  struct kolibri_callback *next_cb;
};

struct kolibri_callback *kcb_head = calloc(sizeof(struct kolibri_callback));
kcb_head -> next_cb = NULL;
struct kolibri_callback *kcb_tail = calloc(sizeof(struct kolibri_callback));

static struct gui_browser_table kolibri_browser_table = {
    .schedule = kolibri_schedule_cb,
    .quit = kolibri_browser_quit,
    };

nserror kolibri_schedule_cb(int t, void (*callback)(void *p), void *p)
{
  struct kolibri_callback *new_cb = malloc(sizeof(struct kolibri_callback));
  new_cb->arg = p;
  new_cb->cb = callback;
  new_cb->next_cb = NULL;

  kcb_tail -> next_cb = new_cb;
  kcb_tail = next_cb;

  return NSERROR_OK;
}

bool kolibri_schedule_run(void)
{
  struct kolibri_callback *runner = kcb_head -> next_cb;
  while(runner != NULL)
    {
      runner->cb(runner->arg);
      runner = runner -> next_cb;
    }
  /* TODO: Free the whole list of callbacks */
}

void kolibri_browser_quit(void)
{
  debug_board_write_str("Netsurf is shutting down Normally..GoodBye.");
  /* TODO: Do the actual killing of process with mcall -1 */
}
