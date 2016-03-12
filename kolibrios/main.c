#include <stdbool.h>
#include <sys/types.h>

#include "kolibrios/kolibri_debug.h"
#include "kolibrios/kolibri_gui.h"

#include "utils/errors.h"

#include "utils/messages.h"
#include "utils/filepath.h"
#include "utils/nsoption.h"
#include "utils/nsurl.h"

#include "desktop/gui_table.h" /* netsurf_table struct */
#include "desktop/gui_misc.h" /* gui_browser_table struct */
#include "desktop/gui_window.h" /* gui_window_table struct */
#include "desktop/netsurf.h" /* for netsurf_register */
#include "image/bitmap.h" /* for gui_bitmap_table struct */
#include "desktop/gui_clipboard.h" /* gui_clipboard_table struct */
#include "desktop/gui_download.h" /* gui_download_table struct */
#include "desktop/gui_fetch.h" /* gui_fetch_table struct */
#include "desktop/browser.h"

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

char **respaths; /** resource search path vector */

/* Inspired from monkey, but without the GTK bloat */
static char **
nskolibri_init_resource(const char *resource_path)
{
  char **langv = {"de", "en_US", "en", "C"};
  char **pathv; /* resource path string vector */
  char **respath; /* resource paths vector */

  pathv = filepath_path_to_strvec(resource_path);
  respath = filepath_generate(pathv, langv);
  filepath_free_strvec(pathv);

  return respath;
}

static nserror set_defaults(struct nsoption_s *defaults)
{
  /* Set defaults for absent option strings */
  nsoption_setnull_charp(cookie_file, strdup("/usbhd0/1/Cookies"));
  nsoption_setnull_charp(cookie_jar, strdup("/usbhd0/1/Cookies"));
  //TODO: This creates an nsoptions error in macro expansion...
  //  nsoption_setnull_charp(url_file, strdup("/usbhd0/1/URLs"));

  return NSERROR_OK;
}

/**
 * Main entry point from Kolibri OS.
 */
extern struct http_msg;

int main(int argc, char** argv)
    {
    nsurl *initial_url;  
    nserror ret;
    unsigned int os_event = KOLIBRI_EVENT_REDRAW;

    /*Also makes sense to have a kolibri_init_libraries(); here which inits all the libraries we'll need */

    /* Need to handle the cookie jar somehow */
    /* First off, get the frontend connected with render engine of NS */
    debug_board_write_str("Netsurf: Official port for KolibriOS.\n");
      
    struct netsurf_table nskolibri_table = {
	/* Tables in Use */
        /* Mandatory tables (From Monkey frontend) */
	.browser = &kolibri_browser_table,
	.window = &kolibri_window_table,
	.download = &kolibri_download_table,
	.fetch = &kolibri_fetch_table,
	.bitmap = &kolibri_bitmap_table,

        /* Optional tables (Not in Monkey, so I assumed optional) */
	/* TODO: Use them in the future, comment out for now for easy debug */
	/* .clipboard = &kolibri_clipboard_table, */
	/* .file = &kolibri_gui_file_table, */
	/* .utf8 = &kolibri_gui_utf8_table, */
	/* .search = &kolibri_gui_search_table, */
	/* .search_web = &kolibri_search_web_table, */
	/* .llcache = &kolibri_gui_llcache_table, */
	};

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
    ret = kolibri_gui_init();
    if (ret == 0)
      debug_board_write_str("Netsurf: KolibriOS BOXLIB Library Initialized.\n");
    else {
      debug_board_write_str("Netsurf: BOXLIB Library initialization failed..\n");
      return ret;
    }      

    /* End of KolibriOS specific libraries initialization phase */
    debug_board_write_str("Netsurf: Trying to register nskolibri_table.\n");
    ret = netsurf_register(&nskolibri_table);

    if(ret == NSERROR_OK)
      debug_board_write_str("Netsurf for KolibriOS: Core Table Initialization Successful.\n");
    else {
      debug_board_write_str("Netsurf: Fatal ERROR -> Core Table Initialization Failed.\n");
      return ret;
    }

    respaths = nskolibri_init_resource("/usbhd0/1/gtk/res");
    debug_board_write_str("Initializing Netsurf Core.");

    ret = nsoption_init(set_defaults, &nsoptions, &nsoptions_default);

    if (ret != NSERROR_OK) {
      debug_board_write_str("Options failed to initialise\n");
      return -2;
    }

    ret = netsurf_init(NULL);
    if(ret == NSERROR_OK)
      debug_board_write_str("Successful!. Exiting Netsurf Gracefully.\n");
    else {
      debug_board_write_str("Failed. Netsurf Abnormal Termination.\n");
      return ret;
    }

    if (nsurl_create("http://www.kolibrios.org", &initial_url) == NSERROR_OK)
      {
	nserror error = browser_window_create(BW_CREATE_HISTORY,
				      initial_url,
				      NULL,
				      NULL,
				      NULL);
	nsurl_unref(initial_url);
      }
    else
      {
	debug_board_write_str("Failed to create initial URL for Netsurf. Exiting.\n");
	return 2;
      }

  do  /* Start of main activity loop */
    {
      char event[20];
      sprintf(event, "ev: %u", os_event);
      debug_board_write_str(event);
    } while(os_event = get_os_event()); /* End of main activity loop */

    return 0;
    }

/* End of main() */
