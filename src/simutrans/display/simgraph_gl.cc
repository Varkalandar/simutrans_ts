/*
 * This file is part of the Simutrans project under the Artistic License.
 * (see LICENSE.txt)
 */


// #include "../simconst.h"
#include "../simmem.h"
#include "../sys/simsys.h"
#include "../simdebug.h"
#include "../descriptor/image.h"

#include "simgraph.h"
#include "rgba.h"
#include "font.h"
#include "gl_textures.h"

#include <GLFW/glfw3.h>


static GLFWwindow* window;
static GLint gl_max_texture_size;


// we try to collect and combine images into large
// tile sheets to minimize texture switching.
static gl_texture_t * gl_texture_sheets[256];
static int gl_current_sheet;
static int gl_current_sheet_x;
static int gl_current_sheet_y;


scr_coord_val tile_raster_width = 16; // zoomed
scr_coord_val base_tile_raster_width = 16; // original
scr_coord_val current_tile_raster_width = 0;


static scr_coord_val display_width;
static scr_coord_val display_height;

static clip_dimension clip_rect;
static clip_dimension clip_rect_swap;
static bool swap_active = 0;

#define TRANSPARENT_RUN (0x8000u)

struct imd_t {
	sint16 x; // current (zoomed) min x offset
	sint16 y; // current (zoomed) min y offset
	sint16 w; // current (zoomed) width
	sint16 h; // current (zoomed) height

	// uint8 recode_flags;
	// uint16 player_flags; // bit # is player number, ==1 cache image needs recoding

	// PIXVAL* data[MAX_PLAYER_COUNT]; // current data - zoomed and recolored (player + daynight)

	// PIXVAL* zoom_data; // zoomed original data
	uint32 len;    // current zoom image data size (or base if not zoomed) (used for allocation purposes only)

	sint16 base_x; // min x offset
	sint16 base_y; // min y offset
	sint16 base_w; // width
	sint16 base_h; // height

	gl_texture_t * texture;
	sint16 sheet_x;
	sint16 sheet_y;

	uint8_t * base_data; // original image data
};

/*
 * Image table
 */
static struct imd_t* images = NULL;

/*
 * Number of loaded images
 */
static image_id anz_images = 0;

/*
 * Number of allocated entries for images
 * (>= anz_images)
 */
static image_id alloc_images = 0;


// things to get rid off
/*
 * special colors during daytime
 */
rgb888_t display_day_lights[LIGHT_COUNT] = {
	{ 0x57, 0x65, 0x6F }, // Dark windows, lit yellowish at night
	{ 0x7F, 0x9B, 0xF1 }, // Lighter windows, lit blueish at night
	{ 0xFF, 0xFF, 0x53 }, // Yellow light
	{ 0xFF, 0x21, 0x1D }, // Red light
	{ 0x01, 0xDD, 0x01 }, // Green light
	{ 0x6B, 0x6B, 0x6B }, // Non-darkening grey 1 (menus)
	{ 0x9B, 0x9B, 0x9B }, // Non-darkening grey 2 (menus)
	{ 0xB3, 0xB3, 0xB3 }, // non-darkening grey 3 (menus)
	{ 0xC9, 0xC9, 0xC9 }, // Non-darkening grey 4 (menus)
	{ 0xDF, 0xDF, 0xDF }, // Non-darkening grey 5 (menus)
	{ 0xE3, 0xE3, 0xFF }, // Nearly white light at day, yellowish light at night
	{ 0xC1, 0xB1, 0xD1 }, // Windows, lit yellow
	{ 0x4D, 0x4D, 0x4D }, // Windows, lit yellow
	{ 0xE1, 0x00, 0xE1 }, // purple light for signals
	{ 0x01, 0x01, 0xFF }  // blue light
};


/*
 * special colors during nighttime
 */
rgb888_t display_night_lights[LIGHT_COUNT] = {
	{ 0xD3, 0xC3, 0x80 }, // Dark windows, lit yellowish at night
	{ 0x80, 0xC3, 0xD3 }, // Lighter windows, lit blueish at night
	{ 0xFF, 0xFF, 0x53 }, // Yellow light
	{ 0xFF, 0x21, 0x1D }, // Red light
	{ 0x01, 0xDD, 0x01 }, // Green light
	{ 0x6B, 0x6B, 0x6B }, // Non-darkening grey 1 (menus)
	{ 0x9B, 0x9B, 0x9B }, // Non-darkening grey 2 (menus)
	{ 0xB3, 0xB3, 0xB3 }, // non-darkening grey 3 (menus)
	{ 0xC9, 0xC9, 0xC9 }, // Non-darkening grey 4 (menus)
	{ 0xDF, 0xDF, 0xDF }, // Non-darkening grey 5 (menus)
	{ 0xFF, 0xFF, 0xE3 }, // Nearly white light at day, yellowish light at night
	{ 0xD3, 0xC3, 0x80 }, // Windows, lit yellow
	{ 0xD3, 0xC3, 0x80 }, // Windows, lit yellow
	{ 0xE1, 0x00, 0xE1 }, // purple light for signals
	{ 0x01, 0x01, 0xFF }  // blue light
};

// ---


// the players colors and colors for simple drawing operations
// each three values correspond to a color, each 8 colors are a player color
static const rgb888_t special_pal[SPECIAL_COLOR_COUNT] =
{
	{  36,  75, 103 }, {  57,  94, 124 }, {  76, 113, 145 }, {  96, 132, 167 }, { 116, 151, 189 }, { 136, 171, 211 }, { 156, 190, 233 }, { 176, 210, 255 },
	{  88,  88,  88 }, { 107, 107, 107 }, { 125, 125, 125 }, { 144, 144, 144 }, { 162, 162, 162 }, { 181, 181, 181 }, { 200, 200, 200 }, { 219, 219, 219 },
	{  17,  55, 133 }, {  27,  71, 150 }, {  37,  86, 167 }, {  48, 102, 185 }, {  58, 117, 202 }, {  69, 133, 220 }, {  79, 149, 237 }, {  90, 165, 255 },
	{ 123,  88,   3 }, { 142, 111,   4 }, { 161, 134,   5 }, { 180, 157,   7 }, { 198, 180,   8 }, { 217, 203,  10 }, { 236, 226,  11 }, { 255, 249,  13 },
	{  86,  32,  14 }, { 110,  40,  16 }, { 134,  48,  18 }, { 158,  57,  20 }, { 182,  65,  22 }, { 206,  74,  24 }, { 230,  82,  26 }, { 255,  91,  28 },
	{  34,  59,  10 }, {  44,  80,  14 }, {  53, 101,  18 }, {  63, 122,  22 }, {  77, 143,  29 }, {  92, 164,  37 }, { 106, 185,  44 }, { 121, 207,  52 },
	{   0,  86,  78 }, {   0, 108,  98 }, {   0, 130, 118 }, {   0, 152, 138 }, {   0, 174, 158 }, {   0, 196, 178 }, {   0, 218, 198 }, {   0, 241, 219 },
	{  74,   7, 122 }, {  95,  21, 139 }, { 116,  37, 156 }, { 138,  53, 173 }, { 160,  69, 191 }, { 181,  85, 208 }, { 203, 101, 225 }, { 225, 117, 243 },
	{  59,  41,   0 }, {  83,  55,   0 }, { 107,  69,   0 }, { 131,  84,   0 }, { 155,  98,   0 }, { 179, 113,   0 }, { 203, 128,   0 }, { 227, 143,   0 },
	{  87,   0,  43 }, { 111,  11,  69 }, { 135,  28,  92 }, { 159,  45, 115 }, { 183,  62, 138 }, { 230,  74, 174 }, { 245, 121, 194 }, { 255, 156, 209 },
	{  20,  48,  10 }, {  44,  74,  28 }, {  68,  99,  45 }, {  93, 124,  62 }, { 118, 149,  79 }, { 143, 174,  96 }, { 168, 199, 113 }, { 193, 225, 130 },
	{  54,  19,  29 }, {  82,  44,  44 }, { 110,  69,  58 }, { 139,  95,  72 }, { 168, 121,  86 }, { 197, 147, 101 }, { 226, 173, 115 }, { 255, 199, 130 },
	{   8,  11, 100 }, {  14,  22, 116 }, {  20,  33, 139 }, {  26,  44, 162 }, {  41,  74, 185 }, {  57, 104, 208 }, {  76, 132, 231 }, {  96, 160, 255 },
	{  43,  30,  46 }, {  68,  50,  85 }, {  93,  70, 110 }, { 118,  91, 130 }, { 143, 111, 170 }, { 168, 132, 190 }, { 193, 153, 210 }, { 219, 174, 230 },
	{  63,  18,  12 }, {  90,  38,  30 }, { 117,  58,  42 }, { 145,  78,  55 }, { 172,  98,  67 }, { 200, 118,  80 }, { 227, 138,  92 }, { 255, 159, 105 },
	{  11,  68,  30 }, {  33,  94,  56 }, {  54, 120,  81 }, {  76, 147, 106 }, {  98, 174, 131 }, { 120, 201, 156 }, { 142, 228, 181 }, { 164, 255, 207 },
	{  64,   0,   0 }, {  96,   0,   0 }, { 128,   0,   0 }, { 192,   0,   0 }, { 255,   0,   0 }, { 255,  64,  64 }, { 255,  96,  96 }, { 255, 128, 128 },
	{   0, 128,   0 }, {   0, 196,   0 }, {   0, 225,   0 }, {   0, 240,   0 }, {   0, 255,   0 }, {  64, 255,  64 }, {  94, 255,  94 }, { 128, 255, 128 },
	{   0,   0, 128 }, {   0,   0, 192 }, {   0,   0, 224 }, {   0,   0, 255 }, {   0,  64, 255 }, {   0,  94, 255 }, {   0, 106, 255 }, {   0, 128, 255 },
	{ 128,  64,   0 }, { 193,  97,   0 }, { 215, 107,   0 }, { 235, 118,   0 }, { 255, 128,   0 }, { 255, 149,  43 }, { 255, 170,  85 }, { 255, 193, 132 },
	{   8,  52,   0 }, {  16,  64,   0 }, {  32,  80,   4 }, {  48,  96,   4 }, {  64, 112,  12 }, {  84, 132,  20 }, { 104, 148,  28 }, { 128, 168,  44 },
	{ 164, 164,   0 }, { 180, 180,   0 }, { 193, 193,   0 }, { 215, 215,   0 }, { 235, 235,   0 }, { 255, 255,   0 }, { 255, 255,  64 }, { 255, 255, 128 },
	{  32,   4,   0 }, {  64,  20,   8 }, {  84,  28,  16 }, { 108,  44,  28 }, { 128,  56,  40 }, { 148,  72,  56 }, { 168,  92,  76 }, { 184, 108,  88 },
	{  64,   0,   0 }, {  96,   8,   0 }, { 112,  16,   0 }, { 120,  32,   8 }, { 138,  64,  16 }, { 156,  72,  32 }, { 174,  96,  48 }, { 192, 128,  64 },
	{  32,  32,   0 }, {  64,  64,   0 }, {  96,  96,   0 }, { 128, 128,   0 }, { 144, 144,   0 }, { 172, 172,   0 }, { 192, 192,   0 }, { 224, 224,   0 },
	{  64,  96,   8 }, {  80, 108,  32 }, {  96, 120,  48 }, { 112, 144,  56 }, { 128, 172,  64 }, { 150, 210,  68 }, { 172, 238,  80 }, { 192, 255,  96 },
	{  32,  32,  32 }, {  48,  48,  48 }, {  64,  64,  64 }, {  80,  80,  80 }, {  96,  96,  96 }, { 172, 172, 172 }, { 236, 236, 236 }, { 255, 255, 255 },
	{  41,  41,  54 }, {  60,  45,  70 }, {  75,  62, 108 }, {  95,  77, 136 }, { 113, 105, 150 }, { 135, 120, 176 }, { 165, 145, 218 }, { 198, 191, 232 }
};


rgba_t color_idx_to_rgb(int idx)
{
    if(idx >= 0 && idx < SPECIAL_COLOR_COUNT) {
        return special_pal[idx];
    }
    else {
        return RGBA_BLACK;
    }
}


rgba_t color_idx_to_rgba(int idx, int transparent_percent)
{
    if(idx >= 0 && idx < SPECIAL_COLOR_COUNT) {
        return rgba_t(special_pal[idx], 1.0f - (transparent_percent / 100.0f));
    }
    else {
        return RGBA_BLACK;
    }
}


rgb888_t get_color_rgb(uint8 idx)
{
	return special_pal[idx];
}


void env_t_rgb_to_system_colors()
{
}


/**
 * changes the raster width after loading
 */
scr_coord_val display_set_base_raster_width(scr_coord_val new_raster)
{
	const scr_coord_val old = base_tile_raster_width;

	base_tile_raster_width = new_raster;
	tile_raster_width = new_raster;
    current_tile_raster_width = new_raster;

	return old;
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
    return display_width;
}


scr_coord_val display_get_height()
{
    return display_height;
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


static uint8_t * rgb343to888(const uint16_t c, const uint8_t alpha, uint8_t *tp)
{

    *tp++ = (c >> 2) & 0xE0; // red
    *tp++ = (c << 1) & 0xF0; // green
    *tp++ = (c << 5) & 0xE0; // blue
    *tp++ = alpha;           // alpha


/*
    *tp++ = 255; // red
    *tp++ = 255; // green
    *tp++ = 0; // blue
    *tp++ = 255;           // alpha

	dbg->message("rgb343to888", "343 input %x, alpha=%d -> %d %d %d", c, alpha, *(tp-4), *(tp-3), *(tp-2));
*/
    return tp;
}


static uint8_t * rgb555to888(const uint16_t c, const uint8_t alpha, uint8_t *tp)
{
    *tp++ = (c >> 7) & 0xF8; // red
    *tp++ = (c >> 2) & 0xF8; // green
    *tp++ = (c << 3) & 0xF8; // blue
    *tp++ = alpha;           // alpha

    return tp;
}


static uint8_t * special_rgb_to_rgba(const uint16_t c, const uint8_t alpha, uint8_t *tp)
{
    uint32 rgb = image_t::rgbtab[c & 255];
    *tp ++ = (rgb >> 16) & 0xFF;
    *tp ++ = (rgb >> 8) & 0xFF;
    *tp ++ = (rgb >> 0) & 0xFF;
    *tp ++ = alpha;
}


static void convert_transparent_pixel_run(uint8_t * dest, const uint16_t * src, const uint16_t * const end)
{
    if(*src < 0x8000) dbg->message("convert_transparent_pixel_run()", "Found suspicious pixel %x", *src);


	// player color or transparent rgb?
	if (*src < 0x8020) {
		// player or special color
		while (src < end) {
			dest = special_rgb_to_rgba(*src++, 255, dest);;
		}
	}
	else {
		while (src < end) {
			// a semi-transparent pixel

			// v = 0x8020 + 31*31 + pix*31 + alpha;
			uint16 aux   = *src++ - 0x8020 - 31*31;
			uint16 alpha = ((aux % 31) + 1);
			dest = rgb343to888((aux / 31) & 0x3FF, alpha << 3, dest);
		}
	}
}


static uint8_t * convert(const uint16_t * sp, const uint16_t runlen, uint8_t * tp)
{
    // dbg->message("convert()", "Converting a run of %d pixels", runlen);

    const uint16_t * end = sp + runlen;

    while(sp < end)
    {
        uint16_t c = *sp ++;
		if(c >= 0x8000)
		{
			// dbg->message("covert()", "Found suspicious pixel %x", c);
			uint32 rgb = image_t::rgbtab[c & 255];
			*tp ++ = (rgb >> 16) & 0xFF;
			*tp ++ = (rgb >> 8) & 0xFF;
			*tp ++ = (rgb >> 0) & 0xFF;
			*tp ++ = 255;
		}
		else
		{
			tp = rgb555to888(c, 255, tp);
		}
    }

    return tp;
}


static void copy_into_texture_sheet(imd_t * image, uint8_t * tex, int scanline)
{
    for(int y=0; y<image->base_h; y++)
    {
        for(int x=0; x<image->base_w; x++)
        {
            uint8_t * src = image->base_data + y*image->base_w * 4 + x * 4;
            uint8_t * dst = tex +
                            (gl_current_sheet_y+y) * scanline * 4+
                            (gl_current_sheet_x+x) * 4;
            // rgba
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            *dst = *src;
        }
    }
}


static void convert_image(imd_t * image)
{
	// is this an oversized image?
	if(image->base_w > tile_raster_width || image->base_h > tile_raster_width)
	{
		// yes, give it a texture of its own
		image->texture = gl_texture_t::create_texture(image->base_w, image->base_h, image->base_data);
		image->sheet_x = 0;
		image->sheet_y = 0;
	}
	else
	{
		// dbg->message("register_image()", "Advancing in sheet line to x=%d", gl_current_sheet_x);

		// do we need to start a new row?
		if(gl_current_sheet_x + image->base_w > gl_max_texture_size)
		{
			gl_current_sheet_x = 0;
			gl_current_sheet_y += base_tile_raster_width;

			dbg->message("register_image()", "Starting texture sheet line %d of %d pixels", gl_current_sheet_y, gl_max_texture_size);

			// time to start a new sheet?
			if(gl_current_sheet_y >= gl_max_texture_size)
			{
				gl_current_sheet_y = 0;
				gl_current_sheet ++;
			}
		}

		// do we need to allocate a new  sheet?
		if(gl_texture_sheets[gl_current_sheet] == 0)
		{
			dbg->message("register_image()", "Starting texture sheet #%d", gl_current_sheet);
			gl_current_sheet_x = 0;
			gl_current_sheet_y = 0;

			gl_texture_sheets[gl_current_sheet] =
				gl_texture_t::create_texture(gl_max_texture_size, gl_max_texture_size,
											 (uint8_t *)calloc(gl_max_texture_size * gl_max_texture_size * 4, 1));

            free(gl_texture_sheets[gl_current_sheet]->data);
            gl_texture_sheets[gl_current_sheet]->data = 0;
		}

		image->sheet_x = gl_current_sheet_x;
		image->sheet_y = gl_current_sheet_y;
		image->texture = gl_texture_sheets[gl_current_sheet];

		// copy_into_texture_sheet(image, image->texture->data, gl_max_texture_size);
		image->texture->update_region(image->sheet_x, image->sheet_y, image->base_w, image->base_h, image->base_data);

        // advance in row
		gl_current_sheet_x += image->base_w;
	}
}


void register_image(image_t * image_in)
{
	struct imd_t *image;

	/* valid image? */
	if(image_in->len == 0 || image_in->h == 0) {
		dbg->warning("register_image()", "Ignoring image %d because of missing data", anz_images);
		image_in->imageid = IMG_EMPTY;
		return;
	}

	if(anz_images == alloc_images) {
		if(images==NULL) {
			alloc_images = 510;
		}
		else {
			alloc_images += 512;
		}
		if(anz_images > alloc_images) {
			// overflow
			dbg->fatal( "register_image", "*** Out of images (more than %li!) ***", anz_images );
		}
		images = REALLOC(images, imd_t, alloc_images);
	}

	image_in->imageid = anz_images;
	image = &images[anz_images];
	anz_images++;

	dbg->message("register_image()", "Currently at %d images, converting %dx%d pixels", anz_images, image_in->w, image_in->h);

	uint8_t * rgba_data = (uint8_t *)calloc(image_in->w * image_in->h * 4, 1);
    uint8_t * tp = rgba_data;
    const uint16_t * sp = image_in->data;
    scr_coord_val h = image_in->h;

    do { // line decoder
        uint16_t runlen = *sp++;
        uint8_t *p = tp;

        // one line decoder
        do {
            // we start with a clear run
            p += (runlen & ~TRANSPARENT_RUN) * 4;

//            dbg->message("register_image()", "Converting %d transparent pixels", runlen);

            // now get colored pixels
            runlen = *sp++;
            if(runlen & TRANSPARENT_RUN) {
                runlen &= ~TRANSPARENT_RUN;

//                dbg->message("register_image()", "Converting %d special color pixels", runlen);
                convert_transparent_pixel_run(p, sp, sp+runlen);
                p += runlen * 4;
                sp += runlen;
            }
            else {
//                dbg->message("register_image()", "Converting %d color pixels", runlen);

                p = convert(sp, runlen, p);
                sp += runlen;
            }
            runlen = *sp++;
        } while(runlen);

//        dbg->message("register_image()", "-- Line converted, %d left --", h);

        tp += image_in->w * 4;
    } while(--h > 0);

    // debug
/*
    for(int y = 0; y < image_in->h; y++)
    {
        for(int x = 0; x < image_in->w; x++)
        {
            int i = y * image_in->w * 4 + x * 4;

            rgba_data[i] = 64;
            rgba_data[i+1] = (x == 0 || y == 0) ? 255 : 64;
            rgba_data[i+2] = (x == image_in->w-1 || y == image_in->h-1) ? 255 : 64;
            rgba_data[i+3] = 255;
        }
    }
*/

	image->len = image_in->len;
	image->x = image_in->x;
	image->y = image_in->y;
	image->w = image_in->w;
	image->h = image_in->h;
	image->base_x = image_in->x;
	image->base_y = image_in->y;
	image->base_w = image_in->w;
	image->base_h = image_in->h;
	image->base_data = rgba_data;

	convert_image(image);
}


bool display_snapshot(const scr_rect &)
{
	return false;
}


/** query offsets */
void display_get_image_offset(image_id image, scr_coord_val *xoff, scr_coord_val *yoff, scr_coord_val *xw, scr_coord_val *yw)
{
	if(  image < anz_images  ) {
		*xoff = images[image].x;
		*yoff = images[image].y;
		*xw   = images[image].w;
		*yw   = images[image].h;
	}
}


/** query un-zoomed offsets */
void display_get_base_image_offset(image_id image, scr_coord_val *xoff, scr_coord_val *yoff, scr_coord_val *xw, scr_coord_val *yw)
{
	if(  image < anz_images  ) {
		*xoff = images[image].base_x;
		*yoff = images[image].base_y;
		*xw   = images[image].base_w;
		*yw   = images[image].base_h;
	}
}


/**
 * Clips interval [x,x+w] such that left <= x and x+w <= right.
 * If @p w is negative, it stays negative.
 * @returns difference between old and new value of @p x.
 */
inline int clip_intv(scr_coord_val &x, scr_coord_val &w, const scr_coord_val left, const scr_coord_val right)
{
	scr_coord_val xx = min(x+w, right);
	scr_coord_val xoff = left - x;
	if (xoff > 0) { // equivalent to x < left
		x = left;
	}
	else {
		xoff = 0;
	}
	w = xx - x;
	return xoff;
}


/// wrapper for clip_intv
static int clip_wh(scr_coord_val *x, scr_coord_val *w, const scr_coord_val left, const scr_coord_val right)
{
	return clip_intv(*x, *w, left, right);
}


clip_dimension display_get_clip_wh(CLIP_NUM_DEF_NOUSE0)
{
	return clip_rect;
}


void display_set_clip_wh(scr_coord_val x, scr_coord_val y, scr_coord_val w, scr_coord_val h, bool fit)
{
	if (!fit) {
		// clip_wh( &x, &w, 0, display_get_width());
		// clip_wh( &y, &h, 0, display_get_height());
	}
	else {
		clip_wh( &x, &w, clip_rect.x, clip_rect.xx);
		clip_wh( &y, &h, clip_rect.y, clip_rect.yy);
	}

	clip_rect.x = x;
	clip_rect.y = y;
	clip_rect.w = w;
	clip_rect.h = h;
	clip_rect.xx = x + w; // watch out, clips to scr_coord_val max
	clip_rect.yy = y + h; // watch out, clips to scr_coord_val max
}


void display_push_clip_wh(scr_coord_val x, scr_coord_val y, scr_coord_val w, scr_coord_val h)
{
	assert(!swap_active);
	// save active clipping rectangle
	clip_rect_swap = clip_rect;
	// active rectangle provided by parameters
	display_set_clip_wh(x, y, w, h);
	swap_active = true;
}


void display_swap_clip_wh()
{
	if (swap_active) {
		// swap clipping rectangles
		clip_dimension save = clip_rect;
		clip_rect = clip_rect_swap;
		clip_rect_swap = save;
	}
}


void display_pop_clip_wh(CLIP_NUM_DEF0)
{
	if (swap_active) {
		// swap original clipping rectangle back
		clip_rect   = clip_rect_swap;
		swap_active = false;
	}
}


void display_scroll_band(const scr_coord_val, const scr_coord_val, const scr_coord_val)
{
}



static void display_tile_from_sheet(const gl_texture_t * gltex, int x, int y, int w, int h,
                                    int tile_x, int tile_y, int tile_w, int tile_h)
{
    // texture coordinates in fractions of sheet size
    const float left = tile_x / (float)gltex->width;
    const float top = tile_y / (float)gltex->height;
    const float gw = tile_w / (float)gltex->width;
    const float gh = tile_h / (float)gltex->height;

    gl_texture_t::bind(gltex->tex_id);

	glBegin(GL_QUADS);

    glTexCoord2f(left, top);
	glVertex2i(x, y);

    glTexCoord2f(left + gw, top);
	glVertex2i(x + w, y);

    glTexCoord2f(left + gw, top + gh);
	glVertex2i(x + w, y + h);

    glTexCoord2f(left, top + gh);
	glVertex2i(x, y + h);

	glEnd();
}


static void display_box_wh(int x, int y, int w, int h, rgba_t color)
{
	display_fillbox_wh_rgb(x, y, w, 1, color, true);
	display_fillbox_wh_rgb(x, y+h-1, w, 1, color, true);
}


void display_color_img(const image_id id, scr_coord_val x, scr_coord_val y, const sint8 player_nr, scr_coord_val w, scr_coord_val h)
{
	if(id < anz_images)
	{
		// debug clipping
		// display_box_wh(clip_rect.x, clip_rect.y, clip_rect.w, clip_rect.h, rgba_t(1, 0, 0, 0.5f));

	    glScissor(clip_rect.x, display_get_height()-clip_rect.y-clip_rect.h, clip_rect.w, clip_rect.h);
		// glScissor(clip_rect.x, clip_rect.y, clip_rect.w, clip_rect.h);
		glEnable(GL_SCISSOR_TEST);

		imd_t & imd = images[id];

		x += imd.base_x;
		y += imd.base_y;

		w = (w == 0) ? imd.base_w : w;
		h = (h == 0) ? imd.base_h : h;

		glColor4f(1, 1, 1, 1);

		display_tile_from_sheet(imd.texture, x, y, w, h,
								imd.sheet_x, imd.sheet_y, imd.base_w, imd.base_h);

		glDisable(GL_SCISSOR_TEST);
	}
}


void display_base_img(const image_id id, scr_coord_val x, scr_coord_val y, const sint8 player_nr, const bool, const bool  CLIP_NUM_DEF_NOUSE)
{
    display_color_img(id, x, y, player_nr);
}


// local helper function for tiles buttons
static void display_three_image_row( image_id i1, image_id i2, image_id i3, scr_rect row, rgba_t)
{
// 	dbg->message("display_three_image_row", "%d %d %d %d", row.x, row.y, row.w, row.h);


	if(  i1!=IMG_EMPTY  ) {
		scr_coord_val w = images[i1].w;
		display_color_img(i1, row.x, row.y, 0);
		row.x += w;
		row.w -= w;
	}
	// middle
	if(  i2!=IMG_EMPTY  ) {
		scr_coord_val w = images[i2].w;
		// tile it wide
		while(  w <= row.w  ) {
			display_color_img(i2, row.x, row.y, 0);
			row.x += w;
			row.w -= w;
		}
		// for the rest we have to clip the rectangle
		if(  row.w > 0  ) {
			display_color_img(i2, row.x, row.y, 0);
		}
	}
	// right
	if(  i3!=IMG_EMPTY  ) {
		scr_coord_val w = images[i3].w;
		display_color_img(i3, row.get_right()-w, row.y, 0);
		row.w -= w;
	}
}


static scr_coord_val get_img_width(image_id img)
{
	return img != IMG_EMPTY ? images[ img ].w : 0;
}


static scr_coord_val get_img_height(image_id img)
{
	return img != IMG_EMPTY ? images[ img ].h : 0;
}


typedef void (*DISP_THREE_ROW_FUNC)(image_id, image_id, image_id, scr_rect, rgba_t);

/**
 * Base method to display a 3x3 array of images to fit the scr_rect.
 * Special cases:
 * - if images[*][1] are empty, display images[*][0] vertically aligned
 * - if images[1][*] are empty, display images[0][*] horizontally aligned
 */
static void display_img_stretch_intern( const stretch_map_t &imag, scr_rect area, DISP_THREE_ROW_FUNC display_three_image_rowf, rgba_t color)
{
	clip_dimension clip = display_get_clip_wh();
 	display_set_clip_wh(area.x, area.y, area.w, area.h, false);

// 	dbg->message("display_set_clip_wh", "%d %d %d %d", area.x, area.y, area.w, area.h);

	scr_coord_val h_top    = max(max( get_img_height(imag[0][0]), get_img_height(imag[1][0])), get_img_height(imag[2][0]));
	scr_coord_val h_middle = max(max( get_img_height(imag[0][1]), get_img_height(imag[1][1])), get_img_height(imag[2][1]));
	scr_coord_val h_bottom = max(max( get_img_height(imag[0][2]), get_img_height(imag[1][2])), get_img_height(imag[2][2]));


	// center vertically if images[*][1] are empty, display images[*][0]
	if(  imag[0][1] == IMG_EMPTY  &&  imag[1][1] == IMG_EMPTY  &&  imag[2][1] == IMG_EMPTY  ) {
		scr_coord_val h = max(h_top, get_img_height(imag[1][1]));
		// center vertically
		area.y += (area.h-h)/2;
	}

	// center horizontally if images[1][*] are empty, display images[0][*]
	if(  imag[1][0] == IMG_EMPTY  &&  imag[1][1] == IMG_EMPTY  &&  imag[1][2] == IMG_EMPTY  ) {
		scr_coord_val w_left = max(max( get_img_width(imag[0][0]), get_img_width(imag[0][1])), get_img_width(imag[0][2]));
		// center vertically
		area.x += (area.w-w_left)/2;
	}

	// top row
	display_three_image_rowf( imag[0][0], imag[1][0], imag[2][0], area, color);


	// now stretch the middle
	if(  h_middle > 0  ) {
		scr_rect row( area.x, area.y+h_top, area.w, area.h-h_top-h_bottom);
		// tile it wide
		while(  h_middle <= row.h  ) {
			display_three_image_rowf( imag[0][1], imag[1][1], imag[2][1], row, color);
			row.y += h_middle;
			row.h -= h_middle;
		}
		// for the rest we have to clip the rectangle
		if(  row.h > 0  ) {
			display_three_image_rowf( imag[0][1], imag[1][1], imag[2][1], row, color);
		}
	}

	// bottom row
	if(  h_bottom > 0  ) {
		scr_rect row( area.x, area.y+area.h-h_bottom, area.w, h_bottom );
		display_three_image_rowf( imag[0][2], imag[1][2], imag[2][2], row, color);
	}

	display_set_clip_wh(clip.x, clip.y, clip.w, clip.h, false);
}


void display_img_stretch( const stretch_map_t &imag, scr_rect area, rgba_t color )
{
	display_img_stretch_intern(imag, area, display_three_image_row, color);
}


static void display_three_blend_row(image_id i1, image_id i2, image_id i3, scr_rect row, rgba_t color)
{
	if(  i1!=IMG_EMPTY  ) {
		scr_coord_val w = images[i1].w;
		display_rezoomed_img_blend(i1, row.x, row.y, 0, color, false, true);
		row.x += w;
		row.w -= w;
	}
	// right
	if(  i3!=IMG_EMPTY  ) {
		scr_coord_val w = images[i3].w;
		display_rezoomed_img_blend(i3, row.get_right()-w, row.y, 0, color, false, true);
		row.w -= w;
	}
	// middle
	if(  i2!=IMG_EMPTY  ) {
		scr_coord_val w = images[i2].w;
		// tile it wide
		while(  w <= row.w  ) {
			display_rezoomed_img_blend(i2, row.x, row.y, 0, color, false, true);
			row.x += w;
			row.w -= w;
		}
		// for the rest we have to clip the rectangle
		if(  row.w > 0  ) {
			clip_dimension const cl = display_get_clip_wh();
			display_set_clip_wh( cl.x, cl.y, max(0,min(row.get_right(),cl.xx)-cl.x), cl.h );
			display_rezoomed_img_blend(i2, row.x, row.y, 0, color, false, true);
			display_set_clip_wh(cl.x, cl.y, cl.w, cl.h);
		}
	}
}


// this displays a 3x3 array of images to fit the scr_rect like above, but blend the color
void display_img_stretch_blend( const stretch_map_t &imag, scr_rect area, rgba_t color )
{
	// display_img_stretch_intern(imag, area, display_three_blend_row, color);
	display_img_stretch_intern(imag, area, display_three_image_row, color);
}


void display_rezoomed_img_blend(const image_id, scr_coord_val, scr_coord_val, const sint8, rgba_t, const bool, const bool  CLIP_NUM_DEF_NOUSE)
{
}


void display_rezoomed_img_alpha(const image_id, const image_id, const unsigned, scr_coord_val, scr_coord_val, const sint8, rgba_t, const bool, const bool  CLIP_NUM_DEF_NOUSE)
{
}


void display_base_img_blend(const image_id, scr_coord_val, scr_coord_val, const sint8, rgba_t, const bool, const bool  CLIP_NUM_DEF_NOUSE)
{
}


void display_base_img_alpha(const image_id, const image_id, const unsigned, scr_coord_val, scr_coord_val, const sint8, rgba_t, const bool, bool  CLIP_NUM_DEF_NOUSE)
{
}

// variables for storing currently used image procedure set and tile raster width
display_image_proc display_normal = display_base_img;
display_image_proc display_color = display_base_img;
display_blend_proc display_blend = display_base_img_blend;
display_alpha_proc display_alpha = display_base_img_alpha;



rgba_t display_blend_colors(rgba_t c1, rgba_t c2, float mix)
{
	return rgba_t(
        c1.red * (1-mix) + c2.red * mix,
        c1.green * (1-mix) + c2.green * mix,
        c1.blue * (1-mix) + c2.blue * mix
	);
}


void display_blend_wh_rgb(scr_coord_val x, scr_coord_val y, scr_coord_val w, scr_coord_val h, rgba_t color)
{
    glScissor(clip_rect.x, display_get_height()-clip_rect.y-clip_rect.h, clip_rect.w, clip_rect.h);
	glEnable(GL_SCISSOR_TEST);

	gl_texture_t::bind(0);

	glBegin(GL_QUADS);

	glColor4f(color.red, color.green, color.blue, color.alpha);

	glVertex2i(x, y);
	glVertex2i(x+w, y);
	glVertex2i(x+w, y+h);
	glVertex2i(x, y+h);

	glEnd();

	glDisable(GL_SCISSOR_TEST);
}


void display_fillbox_wh_rgb(scr_coord_val x, scr_coord_val y, scr_coord_val w, scr_coord_val h, rgba_t color, bool )
{
    // dbg->message("display_fillbox_wh_rgb()", "Called %d,%d,%d,%d color %f,%f,%f,%f", x, y, w, h, color.red, color.green, color.blue, color.alpha);

	gl_texture_t::bind(0);

	glBegin(GL_QUADS);

	glColor4f(color.red, color.green, color.blue, color.alpha);

	glVertex2i(x, y);
	glVertex2i(x+w, y);
	glVertex2i(x+w, y+h);
	glVertex2i(x, y+h);

	glEnd();
}


void display_fillbox_wh_clip_rgb(scr_coord_val x, scr_coord_val y, scr_coord_val w, scr_coord_val h, rgba_t color, bool  CLIP_NUM_DEF_NOUSE)
{
    // dbg->message("display_fillbox_wh_clip_rgb()", "Called %d,%d,%d,%d color %f,%f,%f,%f", x, y, w, h, color.red, color.green, color.blue, color.alpha);

    glScissor(clip_rect.x, display_get_height()-clip_rect.y-clip_rect.h, clip_rect.w, clip_rect.h);
    // glScissor(clip_rect.x, clip_rect.y, clip_rect.w, clip_rect.h);
	glEnable(GL_SCISSOR_TEST);

	gl_texture_t::bind(0);

	glBegin(GL_QUADS);

	glColor4f(color.red, color.green, color.blue, color.alpha);

	glVertex2i(x, y);
	glVertex2i(x+w, y);
	glVertex2i(x+w, y+h);
	glVertex2i(x, y+h);

	glEnd();

	glDisable(GL_SCISSOR_TEST);
}


void display_vline_wh_clip_rgb(scr_coord_val x, scr_coord_val y, scr_coord_val h, rgba_t color, bool dirty CLIP_NUM_DEF_NOUSE)
{
  display_fillbox_wh_clip_rgb(x, y, 1, h, color, dirty);
}


void display_array_wh(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, const rgb888_t *data)
{
}


int display_glyph(scr_coord_val x, scr_coord_val y, utf32 c, rgba_t color, const font_t * font  CLIP_NUM_DEF_NOUSE)
{
    const scr_coord_val w = font->get_glyph_width(c);
    const scr_coord_val h = font->get_glyph_height(c);

    // Pixel coordinates of the glyph in the sheet
    const int gnr = font->glyphs[c].sheet_index;
    const int glyph_x = (gnr & 31) * 32;
    const int glyph_y = (gnr / 32) * 32;

    const int gy = y + font->get_glyph_top(c);

    glScissor(clip_rect.x, display_get_height()-clip_rect.y-clip_rect.h, clip_rect.w, clip_rect.h);
    // glScissor(clip_rect.x, clip_rect.y, clip_rect.w, clip_rect.h);
    glEnable(GL_SCISSOR_TEST);

	glColor4f(color.red, color.green, color.blue, color.alpha);

    display_tile_from_sheet(font->glyph_sheet, x, gy, w, h, glyph_x, glyph_y, w, h);

    glDisable(GL_SCISSOR_TEST);

	return font->get_glyph_advance(c);
}


void display_ddd_box_rgb(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, rgba_t, rgba_t, bool)
{
}


void display_ddd_box_clip_rgb(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, rgba_t, rgba_t)
{
}


void display_flush_buffer()
{
    // dbg->debug("display_flush_buffer()", "Called");
/*
    display_set_clip_wh(0, 0, 1024, 1000);

    display_fillbox_wh_rgb(0, 0, 1000, 1000, RGBA_BLACK, true);
    // debug, show all textures
    for(int i = 0; i < anz_images; i++)
    {
        int x = (i % 10) * 64;
        int y = (i / 10) * 64;

        display_color_img(i, x, y, 0);
    }
*/

/*
    display_set_clip_wh(0, 0, 1024, 1000);
    display_fillbox_wh_rgb(0, 0, 1000, 1000, rgba_t(0.5f, 0.5f, 0.5f, 1), true);
    glColor4f(1, 1, 1, 1);
    display_tile_from_sheet(gl_texture_sheets[0], 0, 0, 0, 0, 640, 480);
*/

	glfwSwapBuffers(window);

	static uint32 time;
	uint32 now = dr_time();

	dbg->message("display_flush_buffer()", "Frame time=%dms", now - time);
	time = now;

	// glClear(GL_COLOR_BUFFER_BIT);
}


void display_show_pointer(int v)
{
	dbg->message("display_show_pointer()", "%d", v);
}


void display_set_pointer(int)
{
}


void display_show_load_pointer(int v)
{
	dbg->message("display_show_load_pointer()", "%d", v);
}


// callback refs
void sysgl_cursor_pos_callback(GLFWwindow *window, double x, double y);
void sysgl_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void sysgl_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


bool simgraph_init(scr_size size, sint16)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	window = glfwCreateWindow(size.w, size.h, "Simutrans GL", NULL, NULL);

	dbg->message("simgraph_init()", "GLFW %d,%d -> window: %p", size.w, size.h, window);

	if(window)
	{
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        display_width = (scr_coord_val)width;
        display_height = (scr_coord_val)height;

		display_set_clip_wh(0, 0, display_width, display_height, false);

		glfwMakeContextCurrent(window);

		// enable vsync (1 == next frame)
		glfwSwapInterval(1);

		// event callbacks
        glfwSetCursorPosCallback(window, sysgl_cursor_pos_callback);
        glfwSetMouseButtonCallback(window, sysgl_mouse_button_callback);
		glfwSetScrollCallback(window, sysgl_scroll_callback);

        // 2D Initialization
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_TEXTURE_2D);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        simgraph_resize(size);

        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_max_texture_size);

        // some drivers seems to lie here?
        // smaller textures work better
        if(gl_max_texture_size > 4096) gl_max_texture_size = 4096;

        dbg->message("simgraph_init()", "GLFW max texture size is %d", gl_max_texture_size);
	}

	return window;
}


bool is_display_init()
{
	return window != NULL && images != NULL;
}


void display_free_all_images_above(image_id limit)
{
	dbg->message("display_free_all_images_above()", "starting past %d", limit);
}

void simgraph_exit()
{
	glfwDestroyWindow(window);

	dr_os_close();
}


void simgraph_resize(scr_size size)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, size.w, size.h, 0, 1, -1);
    glViewport(0, 0, size.w, size.h);
}


void display_snapshot()
{
}

void display_direct_line_rgb(const scr_coord_val, const scr_coord_val, const scr_coord_val, const scr_coord_val, rgba_t)
{
}

void display_direct_line_dotted_rgb(const scr_coord_val, const scr_coord_val, const scr_coord_val, const scr_coord_val, const scr_coord_val, const scr_coord_val, rgba_t)
{
}

void display_circle_rgb( scr_coord_val, scr_coord_val, int, const rgba_t)
{
}

void display_filled_circle_rgb( scr_coord_val, scr_coord_val, int, const rgba_t)
{
}

void draw_bezier_rgb(scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, scr_coord_val, const rgba_t, scr_coord_val, scr_coord_val)
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

void display_img_aligned(const image_id n, scr_rect area, int align, sint8 player_nr, bool dirty)
{
	if(  n < anz_images  ) {
		scr_coord_val x,y;

		// align the image horizontally
		x = area.x;
		if(  (align & ALIGN_RIGHT) == ALIGN_CENTER_H  ) {
			x -= images[n].x;
			x += (area.w-images[n].w)/2;
		}
		else if(  (align & ALIGN_RIGHT) == ALIGN_RIGHT  ) {
			x = area.get_right() - images[n].x - images[n].w;
		}

		// align the image vertically
		y = area.y;
		if(  (align & ALIGN_BOTTOM) == ALIGN_CENTER_V  ) {
			y -= images[n].y;
			y += (area.h-images[n].h)/2;
		}
		else if(  (align & ALIGN_BOTTOM) == ALIGN_BOTTOM  ) {
			y = area.get_bottom() - images[n].y - images[n].h;
		}

		display_color_img(n, x, y, player_nr);
	}
}

void display_proportional_ellipsis_rgb(scr_rect, const char *, int, rgba_t, bool, bool, rgba_t)
{
}

image_id get_image_count()
{
	return anz_images;
}

void add_poly_clip(int, int, int, int, int)
{
}

void clear_all_poly_clip()
{
}

void activate_ribi_clip(int)
{
}
