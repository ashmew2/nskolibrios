#include <stdbool.h>
#include <sys/types.h>

#include "kolibrios/kolibri_debug.h"
#include "kolibrios/kolibri_gui.h"

#include "utils/errors.h"

#include "desktop/gui_table.h" /* netsurf_table struct */
#include "desktop/gui_misc.h" /* gui_browser_table struct */
#include "desktop/gui_window.h" /* gui_window_table struct */
#include "desktop/netsurf.h" /* for netsurf_register */
#include "image/bitmap.h" /* for gui_bitmap_table struct */
#include "desktop/gui_clipboard.h" /* gui_clipboard_table struct */
#include "desktop/gui_download.h" /* gui_download_table struct */
#include "desktop/gui_fetch.h" /* gui_fetch_table struct */
#include "utils/file.h" /* gui_file_table struct */
#include "desktop/gui_utf8.h" /* gui_utf8_table struct */
#include "desktop/gui_search.h" /* gui_search_table struct */
#include "desktop/searchweb.h" /* gui_search_web_table struct */
#include "content/backing_store.h" /* gui_llcache_table struct */

#include "kolibrios/browser_table.h"
#include "kolibrios/window_table.h"
#include "kolibrios/download_table.h"
#include "kolibrios/clipboard_table.h"
#include "kolibrios/gui_fetch_table.h"
#include "kolibrios/bitmap_table.h"
#include "kolibrios/gui_file_table.h"
#include "kolibrios/gui_llcache_table.h"
#include "kolibrios/gui_search_table.h"
#include "kolibrios/gui_search_web_table.h"
#include "kolibrios/gui_utf8_table.h"

#include "kolibrios/kolibri_http.h"

/**
 * Main entry point from Kolibri OS.
 */
extern struct http_msg;

int main(int argc, char** argv)
    {
    nserror ret;
    /*Also makes sense to have a kolibri_init_libraries(); here which inits all the libraries we'll need */

    /* Need to handle the cookie jar somehow */
    /* First off, get the frontend connected with render engine of NS */
    debug_board_write_str("Netsurf: Official port for KolibriOS.\n");
      
    struct netsurf_table nskolibri_table = {
	/* Tables in Use */
	.browser = &kolibri_browser_table,
	.window = &kolibri_window_table,
	.download = &kolibri_download_table,
	.clipboard = &kolibri_clipboard_table,
	.fetch = &kolibri_fetch_table,
	.bitmap = &kolibri_bitmap_table,

	.file = &kolibri_gui_file_table,
	.utf8 = &kolibri_gui_utf8_table,
	.search = &kolibri_gui_search_table,
	.search_web = &kolibri_search_web_table,
	.llcache = &kolibri_gui_llcache_table,
	};

    debug_board_write_str("Netsurf: Trying to register nskolibri_table.\n");

    /* Initialize KolibriOS related libraries which Netsurf will use */

    /* Initialize HTTP Library */
    ret = kolibri_http_init();
    if (ret == 0)
      debug_board_write_str("Netsurf: KolibriOS HTTP Library Initialized.\n");
    else {
      debug_board_write_str("Netsurf: HTTP Library initialization failed..\n");
      return ret;
    }      

  /* Initialize BoxLib Library for GUI textboxes, etc */
    ret = kolibri_boxlib_init();
    if (ret == 0)
      debug_board_write_str("Netsurf: KolibriOS BOXLIB Library Initialized.\n");
    else {
      debug_board_write_str("Netsurf: BOXLIB Library initialization failed..\n");
      return ret;
    }      

    /* End of KolibriOS specific libraries initialization phase */

    ret = netsurf_register(&nskolibri_table);
    if(ret == NSERROR_OK)
      debug_board_write_str("Netsurf for KolibriOS: Core Table Initialization Successful.\n");
    else {
      debug_board_write_str("Netsurf: Fatal ERROR -> Core Table Initialization Failed.\n");
      return ret;
    }

    debug_board_write_str("Initializing Netsurf Core.");
    ret = netsurf_init("/rd/0/1/");
    if(ret == NSERROR_OK)
      debug_board_write_str("Successful!.\n");
    else {
      debug_board_write_str("Failed. Aborting.\n");
      return ret;
    }
      
    return 0;
    }

/* End of main() */
