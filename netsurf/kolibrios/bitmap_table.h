/* ------------------------------ */
/* Part about GUI BITMAP TABLE. Will contain all functions required as well. */
/* ------------------------------ */

void *kolibri_create_bitmap(int width, int height, unsigned int state)
{

}


void kolibri_destroy_bitmap(void *bitmap)
{

}


void kolibri_set_bitmap_opaque(void *bitmap, bool opaque)
{

}


void kolibri_get_bitmap_opaque(void *bitmap)
{

}


bool kolibri_test_bitmap_opaque(void *bitmap)
{

}


unsigned char *kolibri_get_imgbuffer_from_bitmap(void *bitmap)
{

}


size_t kolibri_get_numbytes_row_image(void *bitmap)
{

}


int kolibri_get_width_bitmap(void *bitmap)
{

}


int kolibri_get_height_bitmap(void *bitmap)
{

}


size_t kolibri_get_bpp(void *bitmap)
{

}


bool kolibri_save_to_disk_bitmap(void *bitmap, const char *path, unsigned flags)
{

}


void kolibri_mark_modified(void *bitmap)
{

}


nserror kolibri_render_content_bitmap(struct bitmap *bitmap, struct hlcache_handle *content)
{

}

struct gui_bitmap_table kolibri_bitmap_table = {
    .create = kolibri_create_bitmap,
    .destroy = kolibri_destroy_bitmap,
    .set_opaque = kolibri_set_bitmap_opaque,
    .get_opaque = kolibri_get_bitmap_opaque,
    .test_opaque = kolibri_test_bitmap_opaque,
    .get_buffer = kolibri_get_imgbuffer_from_bitmap,
    .get_rowstride = kolibri_get_numbytes_row_image,
    .get_width = kolibri_get_width_bitmap,
    .get_height = kolibri_get_height_bitmap, 
    .get_bpp = kolibri_get_bpp,
    .save = kolibri_save_to_disk_bitmap,
    .modified = kolibri_mark_modified,
    .render = kolibri_render_content_bitmap,
    };
