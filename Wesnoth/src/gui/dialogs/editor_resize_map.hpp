/* $Id: editor_resize_map.hpp 52533 2012-01-07 02:35:17Z shadowmaster $ */
/*
   Copyright (C) 2008 - 2012 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_EDITOR_RESIZE_MAP_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_RESIZE_MAP_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2 {

class ttoggle_button;

class teditor_resize_map : public tdialog
{
public:
	enum EXPAND_DIRECTION {
		EXPAND_BOTTOM_RIGHT,
		EXPAND_BOTTOM,
		EXPAND_BOTTOM_LEFT,
		EXPAND_RIGHT,
		EXPAND_CENTER,
		EXPAND_LEFT,
		EXPAND_TOP_RIGHT,
		EXPAND_TOP,
		EXPAND_TOP_LEFT
	};

	/**
	 * Constructor.
	 *
	 * @param width [in]          The initial width of the map.
	 * @param width [out]         The selected width of the map if the dialog
	 *                            returns @ref twindow::OK undefined otherwise.
	 *
	 * @param height [in]         The initial height of the map.
	 * @param height [out]        The selected height of the map if the dialog
	 *                            returns @ref twindow::OK undefined otherwise.
	 *
	 * @param expand_direction [out]
	 *                            The selected expand direction if the dialog
	 *                            returns  @ref twindow::OK undefined
	 *                            otherwise.
	 *
	 * @param copy_edge_terrain [in]
	 *                            The initial value of the copy edge toggle.
	 * @param copy_edge_terrain [out]
	 *                            The final value of the copy edge toggle if
	 *                            the dialog returns  @ref twindow::OK
	 *                            undefined otherwise.
	 */
	teditor_resize_map(
			  int& width
			, int& height
			, EXPAND_DIRECTION& expand_direction
			, bool& copy_edge_terrain);

	/** The excute function see @ref tdialog for more information. */
	static bool execute(
			  int& width
			, int& height
			, EXPAND_DIRECTION& expand_direction
			, bool& copy_edge_terrain
			, CVideo& video)
	{
		return teditor_resize_map(
				  width
				, height
				, expand_direction
				, copy_edge_terrain).show(video);
	}

private:

	/** The currently selected width. */
	tfield_integer* width_;

	/** The currently selected height. */
	tfield_integer* height_;

	/** The original width. */
	int old_width_;

	/** The original height. */
	int old_height_;

	/** The selected expansion direction. */
	EXPAND_DIRECTION& expand_direction_;

	/**
	 * The toggle buttons show the state of expand_direction_.
	 *
	 * Allows both so select a direction and visually show the effect of the
	 * selection.
	 */
	ttoggle_button* direction_buttons_[9];

	void update_expand_direction(twindow& window);

	void set_direction_icon(int index, std::string icon);

	/** Inherited from tdialog */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};

} // namespace gui2

#endif

