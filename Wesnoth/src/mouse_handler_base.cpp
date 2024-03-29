/* $Id: mouse_handler_base.cpp 52533 2012-01-07 02:35:17Z shadowmaster $ */
/*
   Copyright (C) 2006 - 2012 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playturn Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "mouse_handler_base.hpp"

#include "cursor.hpp"
#include "display.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "tooltips.hpp"
#include "gui/widgets/fakeminimap.hpp"

static lg::log_domain log_display("display");
#define WRN_DP LOG_STREAM(warn, log_display)

namespace events {

command_disabler::command_disabler()
{
	++commands_disabled;
}

command_disabler::~command_disabler()
{
	--commands_disabled;
}

int commands_disabled= 0;

static bool command_active()
{
#ifdef __APPLE__
	return (SDL_GetModState()&KMOD_META) != 0;
#else
	return false;
#endif
}

mouse_handler_base::mouse_handler_base() :
	simple_warp_(false),
	minimap_scrolling_(false),
	inside_minimap_(false),
	dragging_left_(false),
	dragging_started_(false),
	dragging_right_(false),
	drag_from_x_(0),
	drag_from_y_(0),
	drag_from_hex_(),
	last_hex_(),
	show_menu_(false)
{
}

bool mouse_handler_base::is_dragging() const
{
	return dragging_left_ || dragging_right_;
}

void mouse_handler_base::mouse_motion_event(const SDL_MouseMotionEvent& event, const bool browse)
{
	mouse_motion(event.x,event.y, browse);
}

void mouse_handler_base::mouse_update(const bool browse, map_location loc)
{
	int x, y;
	SDL_GetMouseState(&x,&y);
	mouse_motion(x, y, browse, true, loc);
}

bool mouse_handler_base::mouse_motion_default(int x, int y, bool /*update*/)
{
	tooltips::process(x, y);

	if(simple_warp_) {
		return true;
	}
    gui2::fake_minimap fm;

    //Bobby : Christoffer - Emulate mouse_enter & mouse_leave by checking if mouse inside minimap
	if(point_in_rect(x, y, gui().minimap_area())) {
        //updating indicator_rect for fake map
        int ir_w = 40;
        int ir_h = ir_w;
        int ir_x = x - ir_w/2;
        int ir_y = y - ir_h/2;
		fm.set_indicator_rect(ir_x,ir_y,ir_w,ir_h);

        //Signal mouse_enter if this was the first time we entered the minimap
		if(!inside_minimap_){
            //Mouse enter
            eyetracker::interaction_controller::override_indicator_type(eyetracker::interaction_controller::ZOOM);
            eyetracker::interaction_controller::mouse_enter(&fm);
            inside_minimap_ = true;
		}
		//Update indicator rect to current position on minimap
		else{
            eyetracker::interaction_controller::override_indiactor_rect(fm.indicator_rect());
		}
	}
	else if(inside_minimap_){
        //Mouse leave
        eyetracker::interaction_controller::override_indicator_type(eyetracker::interaction_controller::CLOCK);
        eyetracker::interaction_controller::mouse_leave(&fm);
        inside_minimap_ = false;
	}

	if(minimap_scrolling_) {
		//if the game is run in a window, we could miss a LMB/MMB up event
		// if it occurs outside our window.
		// thus, we need to check if the LMB/MMB is still down
		minimap_scrolling_ = ((SDL_GetMouseState(NULL,NULL) & (SDL_BUTTON(1) | SDL_BUTTON(2))) != 0);
		if(minimap_scrolling_) {
			const map_location& loc = gui().minimap_location_on(x,y);
			if(loc.valid()) {
				if(loc != last_hex_) {
					last_hex_ = loc;
					gui().scroll_to_tile(loc,display::WARP,false);
				}
			} else {
				// clicking outside of the minimap will end minimap scrolling
				minimap_scrolling_ = false;
			}
		}
		if(minimap_scrolling_) return true;
	}

	// Fire the drag & drop only after minimal drag distance
	// While we check the mouse buttons state, we also grab fresh position data.
	int mx = drag_from_x_; // some default value to prevent unlikely SDL bug
	int my = drag_from_y_;
	if (is_dragging() && !dragging_started_) {
		if ((dragging_left_ && (SDL_GetMouseState(&mx,&my) & SDL_BUTTON_LEFT) != 0)
		|| (dragging_right_ && (SDL_GetMouseState(&mx,&my) & SDL_BUTTON_RIGHT) != 0)) {
			const double drag_distance = std::pow(static_cast<double>(drag_from_x_- mx), 2)
					+ std::pow(static_cast<double>(drag_from_y_- my), 2);
			if (drag_distance > drag_threshold()*drag_threshold()) {
				dragging_started_ = true;
				cursor::set_dragging(true);
			}
		}
	}
	return false;
}

void mouse_handler_base::mouse_press(const SDL_MouseButtonEvent& event, const bool browse)
{
	if(is_middle_click(event) && !preferences::middle_click_scrolls()) {
		simple_warp_ = true;
	}
	show_menu_ = false;
	map_location loc = gui().hex_clicked_on(event.x,event.y);
	mouse_update(browse, loc);
	int scrollx = 0;
	int scrolly = 0;

	if (is_left_click(event)) {
		if (event.state == SDL_PRESSED) {
			cancel_dragging();
			init_dragging(dragging_left_);
			left_click(event.x, event.y, browse);
		} else if (event.state == SDL_RELEASED) {
			minimap_scrolling_ = false;
			clear_dragging(event, browse);
			left_mouse_up(event.x, event.y, browse);
		}
	} else if (is_right_click(event)) {
		if (event.state == SDL_PRESSED) {
			cancel_dragging();
			init_dragging(dragging_right_);
			right_click(event.x, event.y, browse);
		} else if (event.state == SDL_RELEASED) {
			minimap_scrolling_ = false;
			clear_dragging(event, browse);
			right_mouse_up(event.x, event.y, browse);
		}
	} else if (is_middle_click(event)) {
		if (event.state == SDL_PRESSED) {
			map_location loc = gui().minimap_location_on(event.x,event.y);
			minimap_scrolling_ = false;
			if(loc.valid()) {
				simple_warp_ = false;
				minimap_scrolling_ = true;
				last_hex_ = loc;
				gui().scroll_to_tile(loc,display::WARP,false);
			} else if(simple_warp_) {
				// middle click not on minimap, check gamemap instead
				loc = gui().hex_clicked_on(event.x,event.y);
				if(loc.valid()) {
					last_hex_ = loc;
					gui().scroll_to_tile(loc,display::WARP,false);
				}
			}
		} else if (event.state == SDL_RELEASED) {
			minimap_scrolling_ = false;
			simple_warp_ = false;
		}
	} else if (allow_mouse_wheel_scroll(event.x, event.y)) {
		if (event.button == SDL_BUTTON_WHEELUP) {
			scrolly = - preferences::scroll_speed();
		} else if (event.button == SDL_BUTTON_WHEELDOWN) {
			scrolly = preferences::scroll_speed();
		} else if (event.button == SDL_BUTTON_WHEELLEFT) {
			scrollx = - preferences::scroll_speed();
		} else if (event.button == SDL_BUTTON_WHEELRIGHT) {
			scrollx = preferences::scroll_speed();
		}
	}

	if (scrollx != 0 || scrolly != 0) {
		CKey pressed;
		// Alt + mousewheel do an 90° rotation on the scroll direction
		if (pressed[SDLK_LALT] || pressed[SDLK_RALT])
			gui().scroll(scrolly,scrollx);
		else
			gui().scroll(scrollx,scrolly);
	}
	if (!dragging_left_ && !dragging_right_ && dragging_started_) {
		dragging_started_ = false;
		cursor::set_dragging(false);
	}
	mouse_update(browse, loc);
}

bool mouse_handler_base::is_left_click(
		const SDL_MouseButtonEvent& event) const
{
	return event.button == SDL_BUTTON_LEFT && !command_active();
}

bool mouse_handler_base::is_middle_click(
		const SDL_MouseButtonEvent& event) const
{
	return event.button == SDL_BUTTON_MIDDLE;
}

bool mouse_handler_base::is_right_click(
		const SDL_MouseButtonEvent& event) const
{
	return event.button == SDL_BUTTON_RIGHT
			|| (event.button == SDL_BUTTON_LEFT && command_active());
}

bool mouse_handler_base::allow_mouse_wheel_scroll(int /*x*/, int /*y*/)
{
	return true;
}

bool mouse_handler_base::right_click_show_menu(int /*x*/, int /*y*/, const bool /*browse*/)
{
	return true;
}

bool mouse_handler_base::left_click(int x, int y, const bool /*browse*/)
{
	if(tooltips::click(x,y))
		return true;

	// clicked on a hex on the minimap? then initiate minimap scrolling
	const map_location& loc = gui().minimap_location_on(x, y);
	minimap_scrolling_ = false;
	if(loc.valid()) {
		minimap_scrolling_ = true;
		last_hex_ = loc;
		gui().scroll_to_tile(loc,display::WARP, false);
		return true;
	}
	return false;
}

void mouse_handler_base::left_drag_end(int x, int y, const bool browse)
{
	left_click(x, y, browse);
}

void mouse_handler_base::left_mouse_up(int /*x*/, int /*y*/, const bool /*browse*/)
{
}

bool mouse_handler_base::right_click(int x, int y, const bool browse)
{
	if (right_click_show_menu(x, y, browse)) {
		gui().draw(); // redraw highlight (and maybe some more)
		const theme::menu* const m = gui().get_theme().context_menu();
		if (m != NULL) {
			show_menu_ = true;
		} else {
			WRN_DP << "no context menu found...\n";
		}
		return true;
	}
	return false;
}

void mouse_handler_base::right_drag_end(int /*x*/, int /*y*/, const bool /*browse*/)
{
	//FIXME: This is called when we select an option in context-menu,
	//       which is bad because that was not a real dragging
}

void mouse_handler_base::right_mouse_up(int /*x*/, int /*y*/, const bool /*browse*/)
{
}

void mouse_handler_base::init_dragging(bool& dragging_flag)
{
	dragging_flag = true;
	SDL_GetMouseState(&drag_from_x_, &drag_from_y_);
	drag_from_hex_ = gui().hex_clicked_on(drag_from_x_, drag_from_y_);
}

void mouse_handler_base::cancel_dragging()
{
	dragging_started_ = false;
	dragging_left_ = false;
	dragging_right_ = false;
	cursor::set_dragging(false);
}

void mouse_handler_base::clear_dragging(const SDL_MouseButtonEvent& event, bool browse)
{
	// we reset dragging info before calling functions
	// because they may take time to return, and we
	// could have started other drag&drop before that
	cursor::set_dragging(false);
	if (dragging_started_) {
		dragging_started_ = false;
		if (dragging_left_) {
			dragging_left_ = false;
			left_drag_end(event.x, event.y, browse);
		}
		if (dragging_right_) {
			dragging_right_ = false;
			right_drag_end(event.x, event.y, browse);
		}
	} else {
		dragging_left_ = false;
		dragging_right_ = false;
	}
}


} //end namespace events
