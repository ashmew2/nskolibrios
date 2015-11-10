/* ------------------------------ */
/* Part about GUI SEARCH TABLE. Will contain all functions required as well. */
/* ------------------------------ */

void kolibri_status(bool found, void *p)
{

}

void kolibri_hourglass(bool active, void *p)
{

}

void kolibri_add_recent(const char *string, void *p)
{

}

void kolibri_forward_state(bool active, void *p)
{

}

void kolibri_back_state(bool active, void *p)
{

}



struct gui_search_table kolibri_gui_search_table = {
   .status = kolibri_status,
   .hourglass = kolibri_hourglass,
   .add_recent = kolibri_add_recent,
   .forward_state = kolibri_forward_state,
   .back_state = kolibri_back_state
};
