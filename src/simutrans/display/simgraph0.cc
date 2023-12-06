/*
 * This file is part of the Simutrans project under the Artistic License.
 * (see LICENSE.txt)
 */

#include "../simconst.h"
#include "../sys/simsys.h"
#include "../descriptor/image.h"

#include "simgraph.h"


scr_coord_val tile_raster_width = 16; // zoomed
scr_coord_val base_tile_raster_width = 16; // original


rgba_t color_idx_to_rgb(PIXVAL idx)
{
	return idx;
}

PIXVAL color_rgb_to_idx(PIXVAL color)
{
	return color;
}


rgb888_t get_color_rgb(uint8)
{
	return {0,0,0};
}

void env_t_rgb_to_system_colors()
{
}

scr_coord_val display_set_base_raster_width(scr_coord_val)
{
	return 0;
}

void set_zoom_factor(int)
{
}

int zoom_factor_up()
{
	return false;
}

int zoom_factor_down()
{
	return false;
}

void mark_rect_dirty_wc(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val)
{
}

void mark_rect_dirty_clip(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val  CLIP_NUM_DEF_NOUSE)
{
}

void mark_screen_dirty()
{
}

void display_mark_img_dirty(image_id, scr_coord_val, scr_coord_val)
{
}

scr_coord_val display_get_width()
{
	return 0;
}

scr_coord_val display_get_height()
{
	return 0;
}

void display_set_height(scr_coord_val)
{
}

void display_set_actual_width(scr_coord_val)
{
}

void display_day_night_shift(int)
{
}

void display_set_player_color_scheme(const int, const uint8, const uint8)
{
}

void register_image(image_t* image)
{
	image->imageid = 1;
}

bool display_snapshot(const scr_rect &)
{
	return false;
}

void display_get_image_offset(image_id image, scr_coord_val *xoff, scr_coord_val *yoff, scr_coord_val *xw, scr_coord_val *yw)
{
	if(  image < 2  ) {
		// initialize offsets with dummy values
		*xoff = 0;
		*yoff = 0;
		*xw   = 0;
		*yw   = 0;
	}
}

void display_get_base_image_offset(image_id image, scr_coord_val *xoff, scr_coord_val *yoff, scr_coord_val *xw, scr_coord_val *yw)
{
	if(  image < 2  ) {
		// initialize offsets with dummy values
		*xoff = 0;
		*yoff = 0;
		*xw   = 0;
		*yw   = 0;
	}
}

clip_dimension display_get_clip_wh(CLIP_NUM_DEF_NOUSE0)
{
	clip_dimension clip_rect;
	clip_rect.x = 0;
	clip_rect.xx = 0;
	clip_rect.w = 0;
	clip_rect.y = 0;
	clip_rect.yy = 0;
	clip_rect.h = 0;
	return clip_rect;
}

void display_set_clip_wh(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val  CLIP_NUM_DEF_NOUSE, bool)
{
}

void display_push_clip_wh(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val  CLIP_NUM_DEF_NOUSE)
{
}

void display_swap_clip_wh(CLIP_NUM_DEF_NOUSE0)
{
}

void display_pop_clip_wh(CLIP_NUM_DEF_NOUSE0)
{
}

void display_scroll_band(const scr_coord_val, const scr_coord_val, const scr_coord_val)
{
}

void display_img_aux(const image_id, scr_coord_val, scr_coord_val, const sint8, const bool, const bool  CLIP_NUM_DEF_NOUSE)
{
}

void display_color_img(const image_id, scr_coord_val, scr_coord_val, const sint8, const bool, const bool  CLIP_NUM_DEF_NOUSE)
{
}

void display_base_img(const image_id, scr_coord_val, scr_coord_val, const sint8, const bool, const bool  CLIP_NUM_DEF_NOUSE)
{
}

void display_fit_img_to_width( const image_id, sint16)
{
}

void display_img_stretch(const stretch_map_t &, scr_rect, rgba_t)
{
}

void display_img_stretch_blend(const stretch_map_t &, scr_rect, rgba_t)
{
}

void display_rezoomed_img_blend(const image_id, scr_coord_val, scr_coord_val, const sint8, const rgba_t, const bool, const bool  CLIP_NUM_DEF_NOUSE)
{
}

void display_rezoomed_img_alpha(const image_id, const image_id, const unsigned, scr_coord_val, scr_coord_val, const sint8, const rgba_t, const bool, const bool  CLIP_NUM_DEF_NOUSE)
{
}

void display_base_img_blend(const image_id, scr_coord_val, scr_coord_val, const sint8, const rgba_t, const bool, const bool  CLIP_NUM_DEF_NOUSE)
{
}

void display_base_img_alpha(const image_id, const image_id, const unsigned, scr_coord_val, scr_coord_val, const sint8, const rgba_t, const bool, bool  CLIP_NUM_DEF_NOUSE)
{
}

// variables for storing currently used image procedure set and tile raster width
display_image_proc display_normal = display_base_img;
display_image_proc display_color = display_base_img;
display_blend_proc display_blend = display_base_img_blend;
display_alpha_proc display_alpha = display_base_img_alpha;

signed short current_tile_raster_width = 0;

rgba_t display_blend_colors(rgba_t, rgba_t, int)
{
	return 0;
}


void display_blend_wh_rgb(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, rgba_t, int )
{
}


void display_fillbox_wh_rgb(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, rgba_t, bool )
{
}


void display_fillbox_wh_clip_rgb(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, rgba_t, bool  CLIP_NUM_DEF_NOUSE)
{
}

void display_vline_wh_clip_rgb(scr_coord_val, scr_coord_val, scr_coord_val, rgba_t, bool  CLIP_NUM_DEF_NOUSE)
{
}

void display_array_wh(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, const rgba_t *)
{
}

int display_glyph(scr_coord_val x, scr_coord_val y, utf32 c, control_alignment_t flags, rgba_t default_color, const font_t * font  CLIP_NUM_DEF_NOUSE)
{
	return 0;
}

void display_ddd_box_rgb(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, rgba_t, rgba_t, bool)
{
}

void display_ddd_box_clip_rgb(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, rgba_t, rgba_t)
{
}

void display_ddd_proportional_clip(scr_coord_val, scr_coord_val, rgba_t, rgba_t, const char *, int  CLIP_NUM_DEF_NOUSE)
{
}

void display_flush_buffer()
{
}

void display_show_pointer(int)
{
}

void display_set_pointer(int)
{
}

void display_show_load_pointer(int)
{
}

bool simgraph_init(scr_size, sint16)
{
	return true;
}

bool is_display_init()
{
	return false;
}

void display_free_all_images_above(image_id)
{
}

void simgraph_exit()
{
	dr_os_close();
}

void simgraph_resize(scr_size)
{
}

void display_snapshot()
{
}

void display_direct_line_rgb(const scr_coord_val, const scr_coord_val, const scr_coord_val, const scr_coord_val, const rgba_t)
{
}

void display_direct_line_dotted_rgb(const scr_coord_val, const scr_coord_val, const scr_coord_val, const scr_coord_val, const scr_coord_val, const scr_coord_val, const rgba_t)
{
}

void display_circle_rgb( scr_coord_val, scr_coord_val, int, const rgba_t )
{
}

void display_filled_circle_rgb( scr_coord_val, scr_coord_val, int, const rgba_t )
{
}

void draw_bezier_rgb(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, const rgba_t, scr_coord_val, scr_coord_val )
{
}

void display_right_triangle_rgb(scr_coord_val, scr_coord_val, scr_coord_val, const rgba_t, const bool)
{
}

void display_signal_direction_rgb( scr_coord_val, scr_coord_val, uint8, uint8, rgba_t, rgba_t, bool, uint8 )
{
}

void display_set_progress_text(const char *)
{
}

void display_progress(int, int)
{
}

void display_img_aligned(const image_id, scr_rect, int, sint8, bool)
{
}

void display_proportional_ellipsis_rgb(scr_rect, const char *, int, rgba_t, bool, bool, rgba_t)
{
}

image_id get_image_count()
{
	return 0;
}

#ifdef MULTI_THREAD
void add_poly_clip(int, int, int, int, int  CLIP_NUM_DEF_NOUSE)
{
}

void clear_all_poly_clip(const sint8)
{
}

void activate_ribi_clip(int  CLIP_NUM_DEF_NOUSE)
{
}
#else
void add_poly_clip(int, int, int, int, int)
{
}

void clear_all_poly_clip()
{
}

void activate_ribi_clip(int)
{
}
#endif
