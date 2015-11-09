//Refer ~/dev-netsurf/workspace/netsurf/desktop/gui_window.h

/* ------------------------------ */
/* Part about GUI WINDOW table. Will contain all functions required as well. */
/* Only implement required functions right now. Optional later. */
/* ------------------------------ */

/* Represents a tab */
struct gui_window {
  struct browser_window *root;

  char *title;
  char *url;

  struct gui_window *next, *prev;
};

struct gui_window * kolibri_create_window(struct browser_window *bw, struct gui_window *existing, gui_window_create_flags flags);

void *kolibri_destroy_window(struct gui_window *gw);

void *kolibri_redraw(struct gui_window *g);
void *kolibri_partial_redraw(struct gui_window *g, const struct rect *rect);

bool kolibri_get_scroll(struct gui_window *g, int *sx, int *sy);
void kolibri_set_scroll(struct gui_window *g, int sx, int sy);

void kolibri_get_content_dimensions(struct gui_window *g, int *width, int *height, bool scaled);

void kolibri_update_extent(struct gui_window *g);
void kolibri_reformat_contents(struct gui_window *g); /* Invoke libimg and reformat stuff in the contents area here */

static struct gui_window_table kolibri_window_table = {
    .create = kolibri_create_window,
    .destroy = kolibri_destroy_window,
    .redraw = kolibri_redraw,
    .update = kolibri_partial_redraw,
    .get_scroll = kolibri_get_scroll,
    .set_scroll = kolibri_set_scroll,
    .get_dimensions = kolibri_get_content_dimensions,
    .update_extent = kolibri_update_extent,
    .reformat = kolibri_reformat_contents,
    };

struct gui_window * kolibri_create_window(struct browser_window *bw, struct gui_window *existing, gui_window_create_flags flags)
{
  struct gui_window *new_tab_window = (struct gui_window *)malloc(sizeof(struct gui_window));
  new_tab_window->root = bw;
  new_tab_window->url = NULL;
  new_tab_window->title = NULL;
  
  if(existing)
    new_tab_window->prev = existing;
  else
    new_tab_window->prev = NULL;
  
  new_tab_window->next = NULL;
  
  return new_tab_window;
}

void *kolibri_destroy_window(struct gui_window *gw)
{
  free(gw);
}

void *kolibri_redraw(struct gui_window *g)
{
  // SF 12,1
}

void *kolibri_partial_redraw(struct gui_window *g, const struct rect *rect)
{
  // SF 12,1 for now
}

bool kolibri_get_scroll(struct gui_window *g, int *sx, int *sy)
{
  // SF 37,7
}

void kolibri_set_scroll(struct gui_window *g, int sx, int sy)
{
  
}

void kolibri_get_content_dimensions(struct gui_window *g, int *width, int *height, bool scaled)
{
  /* Return some part of the browser area resolution.
     If the browser is ALWAYS MAXIMIZED, return the Length x Height minus toolbar , url bar, tab area
  */
}


void kolibri_update_extent(struct gui_window *g)
{
  /* Not sure what to do, but some libimg magic here */
}


void kolibri_reformat_contents(struct gui_window *g)
{
    /* Not sure what to do, but some libimg magic here */
}

 /* Invoke libimg and reformat stuff in the contents area here */

