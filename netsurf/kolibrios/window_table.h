//Refer ~/dev-netsurf/workspace/netsurf/desktop/gui_window.h

/* ------------------------------ */
/* Part about GUI WINDOW table. Will contain all functions required as well. */
/* Only implement required functions right now. Optional later. */
/* ------------------------------ */

struct gui_window {
  struct browser_window *root;

  char *title;
  char *url;
  void *kobject; /* Kolibri GUI Window for now */
  
  struct gui_window *next, *prev;
};

struct gui_window *master_window;

struct gui_window* kolibri_create_window(struct browser_window *bw, struct gui_window *existing, gui_window_create_flags flags)
{
  struct gui_window *new_window = (struct gui_window *)malloc(sizeof(struct gui_window));
  
  unsigned int gui_event = KOLIBRI_EVENT_REDRAW;
  struct kolibri_window *main_window = kolibri_new_window(20, 20, 400, 400, "Netsurf: Official Port for KolibriOS.");
  struct edit_box *textbox = kolibri_new_edit_box(30, 30, 40);
  kolibri_window_add_element(main_window, KOLIBRI_EDIT_BOX, textbox);

  debug_board_write_str("Creating New GUI window for Netsurf \n");

  new_window->root = bw;
  new_window->url = NULL;
  new_window->title = NULL;
  new_window->kobject = main_window;

  /* do  /\* Start of main activity loop *\/ */
  /*   { */
  /*     if(gui_event == KOLIBRI_EVENT_REDRAW) */
  /* 	{ */
  /* 	  kolibri_handle_event_redraw(main_window); */
  /* 	} */
  /*     else if(gui_event == KOLIBRI_EVENT_KEY) */
  /* 	{ */
  /* 	  kolibri_handle_event_key(main_window); */
  /* 	} */
  /*     else if(gui_event == KOLIBRI_EVENT_BUTTON) */
  /* 	{ */

  /* 	} */
  /*     else if(gui_event == KOLIBRI_EVENT_MOUSE) */
  /* 	{ */
  /* 	  kolibri_handle_event_mouse(main_window); */
  /* 	} */
  /*   } while(gui_event = get_os_event()); /\* End of main activity loop *\/ */

  /* TODO:Fix this according to flags maybe> */

  if(existing)
    new_window->prev = existing;
  else
    new_window->prev = NULL;
  
  new_window->next = NULL;
  debug_board_write_str("Returning new GUI window to NS\n");

  master_window = new_window;
  return new_window;
}

void kolibri_destroy_window(struct gui_window *gw)
{
  free(gw);
}

void kolibri_redraw(struct gui_window *g)
{
  // SF 12,1
}

void kolibri_partial_redraw(struct gui_window *g, const struct rect *rect)
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
