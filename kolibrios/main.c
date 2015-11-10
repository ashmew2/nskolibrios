#include "kolibri_debug.h"
#include "kolibri_gui.h"

#include "utils/errors.h"

#include "browser_table.h"
#include "window_table.h"
#include "download_table.h"
#include "clipboard_table.h"
#include "gui_fetch_table.h"
#include "bitmap_table.h"

/**
 * Main entry point from Kolibri OS.
 */

int main(int argc, char** argv)
    {

    /*Also makes sense to have a kolibri_init_libraries(); here which inits ALL the libraries we'll need */

    /* Need to handle the cookie jar somehow */
    /* First off, get the frontend connected with render engine of NS */

    struct netsurf_table nskolibri_table = {
	/* Tables in Use */
	.browser = &kolibri_browser_table,
	.window = &kolibri_window_table,
	.download = &kolibri_download_table,
	.clipboard = &kolibri_clipboard_table,
	.fetch = &kolibri_fetch_table,
	.bitmap = &kolibri_bitmap_table,

	/* Tables Not in Use, Currently NULL */
	.file = &kolibri_file_table,
	.utf8 = &kolibri_utf8_table,
	.search = &kolibri_search_table,
	.search_web = &kolibri_search_web_table,
	.llcache = &kolibri_llcache_table,
	};

    ret = netsurf_register(&nskolibri_table);
    
    if(ret != NSERROR_OK)
      debug_board_write_str("NSERROR IS OK!\n\n\n");
    else
      debug_board_write_str("NSERROR -> FAILED TO INIT\n\n");

    return 0;
    }

/* End of main() */

/* ------------------------------ */
/* Part about GUI FILE TABLE. Will contain all functions required as well. */
/* ------------------------------ */

struct gui_file_table kolibri_file_table = NULL;

/* ------------------------------ */
/* Part about GUI UTF8 TABLE. Will contain all functions required as well. */
/* ------------------------------ */

struct gui_utf8_table kolibri_utf8_table = NULL;


/* ------------------------------ */
/* Part about GUI SEARCH TABLE. Will contain all functions required as well. */
/* ------------------------------ */

struct gui_search_table kolibri_search_table = NULL;

/* ------------------------------ */
/* Part about GUI SEARCH WEB TABLE. Will contain all functions required as well. */
/* ------------------------------ */

struct gui_search_web_table kolibri_search_web_table = NULL;

/* ------------------------------ */
/* Part about GUI LLCACHE TABLE. Will contain all functions required as well. */
/* ------------------------------ */

struct gui_llcache_table kolibri_llcache_table = NULL;

