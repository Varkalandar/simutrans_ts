/*
 * This file is part of the Simutrans project under the Artistic License.
 * (see LICENSE.txt)
 */

#ifndef GUI_COMPONENTS_GUI_MAP_PREVIEW_H
#define GUI_COMPONENTS_GUI_MAP_PREVIEW_H


#include "gui_component.h"
#include "../../display/rgba.h"
#include "../../tpl/array2d_tpl.h"

#define MAP_PREVIEW_SIZE_X ((scr_coord_val)(64))
#define MAP_PREVIEW_SIZE_Y ((scr_coord_val)(64))

/**
 * A map preview component
 *
 */
class gui_map_preview_t : public gui_component_t
{

	private:
		array2d_tpl<rgb888_t> *map_data;

	public:
		gui_map_preview_t();

		void set_map_data(array2d_tpl<rgb888_t> *map_data_par) {
			map_data = map_data_par;
		}

		/**
		 * Draws the component.
		 */
		void draw(scr_coord offset) OVERRIDE ;

		scr_size get_min_size() const OVERRIDE { return scr_size(MAP_PREVIEW_SIZE_X, MAP_PREVIEW_SIZE_Y); }

		scr_size get_max_size() const OVERRIDE { return scr_size(MAP_PREVIEW_SIZE_X, MAP_PREVIEW_SIZE_Y); }
};

#endif
