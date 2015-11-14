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

/**
 * Main entry point from Kolibri OS.
 */

int main(int argc, char** argv)
    {
      nserror ret;
      /*Also makes sense to have a kolibri_init_libraries(); here which inits all the libraries we'll need */

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

	.file = &kolibri_gui_file_table,
	.utf8 = &kolibri_gui_utf8_table,
	.search = &kolibri_gui_search_table,
	.search_web = &kolibri_search_web_table,
	.llcache = &kolibri_gui_llcache_table,
	};

    ret = netsurf_register(&nskolibri_table);
    
    if(ret != NSERROR_OK)
      debug_board_write_str("NSERROR IS OK!\n\n\n");
    else
      debug_board_write_str("NSERROR -> FAILED TO INIT\n\n");

    return 0;
    }

/* End of main() */
