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

/* #include "kolibrios/browser_table.h" */
/* #include "kolibrios/window_table.h" */
/* #include "kolibrios/download_table.h" */
/* #include "kolibrios/clipboard_table.h" */
/* #include "kolibrios/gui_fetch_table.h" */
/* #include "kolibrios/bitmap_table.h" */
/* #include "kolibrios/gui_file_table.h" */
/* #include "kolibrios/gui_llcache_table.h" */
/* #include "kolibrios/gui_search_table.h" */
/* #include "kolibrios/gui_search_web_table.h" */
/* #include "kolibrios/gui_utf8_table.h" */

#include "kolibrios/kolibri_http.h"
#include <libnsfb.h>
#include <libnsfb_plot.h>
#include <libnsfb_event.h>
#include <libnsfb_surface.h>

#include "utils/utils.h"
#include "utils/nsoption.h"
#include "utils/filepath.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "desktop/browser.h"
#include "desktop/textinput.h"
#include "desktop/browser_history.h"
#include "desktop/plotters.h"
#include "desktop/gui_window.h"
#include "desktop/gui_misc.h"
#include "desktop/netsurf.h"

#include "framebuffer/gui.h"
#include "framebuffer/fbtk.h"
#include "framebuffer/framebuffer.h"
#include "framebuffer/schedule.h"
#include "framebuffer/findfile.h"
#include "framebuffer/image_data.h"
#include "framebuffer/font.h"
#include "framebuffer/clipboard.h"
#include "framebuffer/fetch.h"
#include "framebuffer/bitmap.h"

/**************************************************
Make life easier with these path specifiers here.
***************************************************/
/* #define KOLIBRI_FONTFILE "/usbhd0/1/kolibrios/res/sans.ttf" */
/* #define KOLIBRI_RESPATH "/usbhd0/1/kolibrios/res/" */
/* #define FILE_KOLIBRI_RESPATH "file:///usbhd0/1/kolibrios/res/" */
/************************************************/

char **respaths; /** resource search path vector */

extern struct gui_browser_table framebuffer_browser_table;
extern struct gui_window_table framebuffer_window_table ;
extern struct gui_clipboard_table *framebuffer_clipboard_table ;
extern struct gui_fetch_table *framebuffer_fetch_table ;
extern struct gui_utf8_table *framebuffer_utf8_table ;
extern struct gui_bitmap_table *framebuffer_bitmap_table ;

extern const char *fename;
extern int febpp;
extern int fewidth;
extern int feheight;
extern const char *feurl;
extern bool nslog_stream_configure(FILE *fptr);
extern struct fbtk_bitmap pointer_image;
extern fbtk_widget_t *fbtk;

static void die(const char *error)
{
	debug_board_write_str(error);
	debug_board_write_str('\n');
	exit(1);
}

/* Inspired from monkey, but without the GTK bloat */
static char **nskolibri_init_resource(const char *resource_path)
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
  nsoption_setnull_charp(cookie_file, *respaths);
  nsoption_setnull_charp(cookie_jar, *respaths);
  //TODO: This creates an nsoptions error in macro expansion...

  return NSERROR_OK;
}

/**
 * Main entry point from Kolibri OS.
 */
extern struct http_msg;

int
main(int argc, char** argv)
{
	struct browser_window *bw;
	char *options;
	char *messages;
	nsurl *url;
	nserror ret;
	nsfb_t *nsfb;

	struct netsurf_table framebuffer_table = {
		.browser = &framebuffer_browser_table,
		.window = &framebuffer_window_table,
		.clipboard = framebuffer_clipboard_table,
		.fetch = framebuffer_fetch_table,
		.utf8 = framebuffer_utf8_table,
		.bitmap = framebuffer_bitmap_table,
	};

    

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

    ret = netsurf_register(&framebuffer_table);
    if (ret != NSERROR_OK) {
      die("NetSurf operation table failed registration");
    }

    /* Drop the last slash if caller provided it. */

    char current_dir[100];
    int i = 0;

    for(i = strlen(argv[0]); i>=0; i--)
      {
	if(argv[0][i]=='/') {
	  strncpy(current_dir, argv[0], i+1);
	  i++;
	  current_dir[i++] = 'r';
	  current_dir[i++] = 'e';
	  current_dir[i++] = 's';
	  current_dir[i++] = '\0';
	  break;
	}
      }


    respaths = (char **) malloc(sizeof(char **));
    *respaths = malloc(i);
    strcpy(*respaths, current_dir);

    debug_board_write_str("CWD: ");
    debug_board_write_str(*respaths);
    debug_board_write_str("\n");

    /* __asm__ __volatile__("int3"); */
    
    /* initialise logging. Not fatal if it fails but not much we
     * can do about it either.
     */
    nslog_init(nslog_stream_configure, &argc, argv);

    /* user options setup */
    ret = nsoption_init(set_defaults, &nsoptions, &nsoptions_default);
    if (ret != NSERROR_OK) {
      die("Options failed to initialise");
    }
    options = filepath_find(respaths, "Choices");
    nsoption_read(options, nsoptions);
    free(options);
    nsoption_commandline(&argc, argv, nsoptions);

    /* message init */
    messages = filepath_find(respaths, "Messages");
    ret = messages_add_from_file(messages);
    free(messages);
    if (ret != NSERROR_OK) {
      debug_board_write_str("Message translations failed to load\n");
    }

    /* common initialisation */
    ret = netsurf_init(NULL);
    if (ret != NSERROR_OK) {
      die("NetSurf failed to initialise");
    }

    /* Override, since we have no support for non-core SELECT menu */
    nsoption_set_bool(core_select_menu, true);
    /* Do we really need to parse command line in KolibriOS?
       Most users will use the GUI to trigger NS .
       TODO: Look at this later.
    */

    /* if (process_cmdline(argc,argv) != true) */
    /*   die("unable to process command line.\n"); */

    /* Move the initialization stuff from process_cmdline() to here */
    /* fename = "sdl"; */

    extern nsfb_surface_rtns_t kolibri_rtns;
    _nsfb_register_surface(NSFB_SURFACE_KOLIBRI, &kolibri_rtns, "kolibri");
    
    /* fewidth = nsoption_int(window_width); */
    /* if (fewidth <= 0) { */
    /*   fewidth = 800; */
    /* } */

    /* feheight = nsoption_int(window_height); */
    /* if (feheight <= 0) { */
    /*   feheight = 600; */
    /* } */

    fename = "kolibri";
    febpp = 32;
    fewidth = 800;
    feheight = 600;

    feurl = "board.kolibrios.org/ucp.php?i=pm&mode=compose";
    nsfb = framebuffer_initialise(fename, fewidth, feheight, febpp);
    if (nsfb == NULL)
      die("Unable to initialise framebuffer");

    framebuffer_set_cursor(&pointer_image);

    if (fb_font_init() == false)
      die("Unable to initialise the font system");

    fbtk = fbtk_init(nsfb);

    fbtk_enable_oskb(fbtk);

    urldb_load_cookies(nsoption_charp(cookie_file));

    /* create an initial browser window */

    debug_board_write_str("calling browser_window_create\n");
    
    ret = nsurl_create(feurl, &url);
    if (ret == NSERROR_OK) {
      ret = browser_window_create(BW_CREATE_HISTORY,
				  url,
				  NULL,
				  NULL,
				  &bw);
      nsurl_unref(url);
    }

    if (ret != NSERROR_OK) {
      warn_user(messages_get_errorcode(ret), 0);
    } else {
      debug_board_write_str("calling framebuffer run\n");

      framebuffer_run();

      debug_board_write_str("framebuffer run returned. \n");

      debug_board_write_str("Killing browser window.\n");
      browser_window_destroy(bw);
    }

    debug_board_write_str("Calling netsurf_exit()\n");
    netsurf_exit();

    if (fb_font_finalise() == false)
      LOG("Font finalisation failed.");

    /* finalise options */
    nsoption_finalise(nsoptions, nsoptions_default);
    
    free(*respaths);
    free(respaths);
    return 0;
}
