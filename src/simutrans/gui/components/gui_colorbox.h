/*
 * This file is part of the Simutrans project under the Artistic License.
 * (see LICENSE.txt)
 */

#ifndef GUI_COMPONENTS_GUI_COLORBOX_H
#define GUI_COMPONENTS_GUI_COLORBOX_H


#include "gui_component.h"
#include "../../display/rgba.h"

/**
 * Draws a simple colored box.
 */
class gui_colorbox_t : public gui_component_t
{
	rgba_t color;

	scr_size max_size;
public:
    scr_coord_val fixed_min_height;

	gui_colorbox_t(rgba_t color=RGBA_BLACK);

	void draw(scr_coord offset) OVERRIDE;

	void set_color(rgba_t c)
	{
            color = c;
	}

	scr_size get_min_size() const OVERRIDE;

	scr_size get_max_size() const OVERRIDE;

	void set_max_size(scr_size s)
	{
            max_size = s;
	}
};

#endif
