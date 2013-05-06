/* $Id: slider.cpp 52533 2012-01-07 02:35:17Z shadowmaster $ */
/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "global.hpp"

#include "widgets/slider.hpp"
#include "game_config.hpp"
#include "font.hpp"
#include "image.hpp"
#include "sound.hpp"
#include "video.hpp"

#include "eyetracker/interaction_controller.hpp"


namespace {
	const std::string slider_image = "buttons/slider.png";
	const std::string selected_image = "buttons/slider-selected.png";
}

namespace gui {

slider::slider(CVideo &video)
	: widget(video), image_(image::get_image(slider_image)),
	  highlightedImage_(image::get_image(selected_image)),
	  min_(-100000), max_(100000), value_(0), xpos_(0),
	  increment_(1), value_change_(false),mouse_entered_(false), state_(NORMAL), steps_(0)
{
}

void slider::enable(bool new_val)
{
	if(new_val != enabled())
	{
		state_ = NORMAL;
		widget::enable(new_val);
	}
}

void slider::set_location(SDL_Rect const &rect)
{
	SDL_Rect r = rect;
	r.h = image_->h;
	widget::set_location(r);
}

void slider::set_min(int value)
{
	min_ = value;
	if (value_ < min_) {
		value_ = min_;
		value_change_ = true;
	}
	set_dirty(true);
}

void slider::set_max(int value)
{
	max_ = value;
	if (value_ > max_) {
		value_ = max_;
		value_change_ = true;
	}
	set_dirty(true);
}

void slider::set_value(int value)
{
	if (value > max_)
		value = max_;
	if (value < min_)
		value = min_;

	if (increment_ > 1) {
		int hi = increment_ / 2;
		value = ((value + hi) / increment_) * increment_;
	}

	if (value == value_)
		return;

	value_ = value;
	value_change_ = true;
	set_dirty(true);
}

void slider::snap_value()
{
	if (xpos_ > max_)
		xpos_ = max_;
	if (xpos_ < min_)
		xpos_ = min_;

	if (increment_ > 1) {
		int hi = increment_ / 2;
		xpos_ = ((xpos_ + hi) / increment_) * increment_;
	}
}

SDL_Rect slider::indicator_rect(){
    if((max_ - min_) < 15) {
        SDL_Rect const &loc = location();
        int tmp = xpos_ - loc.x - image_->w / 2;
        if (tmp < 0)
            tmp = 0;
        if (tmp > loc.w - image_->w)
            tmp = loc.w - image_->w;

        xpos_ = round((0.0 + tmp * (max_ - min_)) / (loc.w - image_->w) + min_);
        snap_value();
        xpos_ = loc.x + (xpos_ - min_) *
            static_cast<int>(loc.w - image_->w) / (max_ - min_);
        int x = xpos_ - slider_area().h/4;
        int y = slider_area().y;
        int w = slider_area().h;
        int h = w;
        return {x,y,w,h};
    }
    else{
        int x = xpos_ - slider_area().h/4;
        int y = slider_area().y;
        int w = slider_area().h;
        int h = w;
        return {x,y,w,h};
    }
}

void slider::set_increment(int increment)
{
	increment_ = increment;
}

int slider::value() const
{
	return value_;
}

int slider::max_value() const
{
	return max_;
}

int slider::min_value() const
{
    return min_;
}

bool slider::value_change()
{
	if (value_change_) {
		value_change_ = false;
		return true;
	}
	return false;
}

SDL_Rect slider::slider_area() const
{
	static const SDL_Rect default_value = {0,0,0,0};
	SDL_Rect const &loc = location();
	if (image_.null() || image_->w >= loc.w)
		return default_value;

	int xpos = loc.x + (value_ - min_) *
		static_cast<int>(loc.w - image_->w) / (max_ - min_);
	return create_rect(xpos, loc.y, image_->w, image_->h);
}

void slider::draw_contents()
{
	surface image(state_ != NORMAL ? highlightedImage_ : image_);
	if (image == NULL)
		return;
	//SDL_Color line_color = font::NORMAL_COLOR;
	SDL_Color line_color_top = {163, 155, 152, 0};
	SDL_Color line_color_bottom = {106, 99, 96, 0};

	if (!enabled()) {
		image = greyscale_image(image);
		line_color_top = line_color_bottom = font::DISABLED_COLOR;
	}

	SDL_Rect const &loc = location();
	if (image->w >= loc.w)
		return;

	surface screen = video().getSurface();

	SDL_Rect line_rect_top = create_rect(loc.x + image->w / 2
			, loc.y + loc.h / 2 - 1
			, loc.w - image->w
			, 1);
    SDL_Rect line_rect_bottom = create_rect(loc.x + image->w / 2
			, loc.y + loc.h / 2
			, loc.w - image->w
			, 1);

	sdl_fill_rect(screen, &line_rect_top, SDL_MapRGB(screen->format,
		line_color_top.r, line_color_top.g, line_color_top.b));
    sdl_fill_rect(screen, &line_rect_bottom, SDL_MapRGB(screen->format,
		line_color_bottom.r, line_color_bottom.g, line_color_bottom.b));

    // Little Bobby: draws steps on certain sliders
    if((max_ - min_) < 15) {
        SDL_Color step_color = {200,200,200,0};
        int width = loc.w - image->w;
        int steps = max_ - min_;
        steps_ = steps;
        int start_x = loc.x + image->w / 2;
        int end_x = start_x + width;
        int step_length = width / steps;
        for(int step = 0; step < steps; step++) {
            SDL_Rect step_rect = create_rect(start_x + step * step_length, loc.y + loc.h/2 - 3, 2, 5);
            sdl_fill_rect(screen, &step_rect, SDL_MapRGB(screen->format,
                step_color.r, step_color.g, step_color.b));
        }
        SDL_Rect step_rect = create_rect(end_x - 1, loc.y + loc.h/2 - 3, 2, 5);
        sdl_fill_rect(screen, &step_rect, SDL_MapRGB(screen->format,
            step_color.r, step_color.g, step_color.b));
    }

	SDL_Rect const &slider = slider_area();
	video().blit_surface(slider.x, slider.y, image);
}

void slider::set_slider_position(int x)
{
	SDL_Rect const &loc = location();
	int tmp = x - loc.x - image_->w / 2;
	if (tmp < 0)
		tmp = 0;
	if (tmp > loc.w - image_->w)
		tmp = loc.w - image_->w;


	set_value( round((0.0 + tmp * (max_ - min_)) / (loc.w - image_->w) + min_) );
}

void slider::mouse_motion(const SDL_MouseMotionEvent& event)
{
    //Is mouse over slider?
    if(point_in_rect(event.x, event.y, location())){
        //Bobby | Christoffer | Added eyetracker support
        if(!mouse_entered_){
            xpos_ = event.x;
            eyetracker::interaction_controller::mouse_enter(this);
            mouse_entered_ = true;
        }
        else{
            //int prevX = event.x;
            //xpos_ = event.x;
            //snap_value();
            if(steps_== 0){
                steps_ = 10;
            }
            std::cerr<<steps_<<std::endl;//", ";
            int change = (this->width()/steps_)/2;
            if(event.x >= xpos_+ change || event.x <= xpos_- change){
               //std::cerr<<increment_<<std::endl;
                //xpos_ = prevX;
                eyetracker::interaction_controller::mouse_leave(this);
                mouse_entered_ = false;
               // eyetracker::interaction_controller::mouse_enter(this);
            }
        }
    }
    else{
        //If we were over an item, mark that we have left the slider
        if(mouse_entered_){
            eyetracker::interaction_controller::mouse_leave(this);
            mouse_entered_ = false;
        }
    }

	if (state_ == NORMAL || state_ == ACTIVE) {
		bool on = point_in_rect(event.x, event.y, slider_area());
		state_ = on ? ACTIVE : NORMAL;
	} else if (state_ == CLICKED || state_ == DRAGGED) {
		state_ = DRAGGED;
		bool prev_change = value_change_;
		value_change_ = false;
		set_slider_position(event.x);
		if(value_change_) {
			sound::play_UI_sound(game_config::sounds::slider_adjust);
		} else {
			value_change_ = prev_change;
		}
	}
}

void slider::mouse_down(const SDL_MouseButtonEvent& event)
{
	if (event.button != SDL_BUTTON_LEFT || !point_in_rect(event.x, event.y, location()))
		return;

	state_ = CLICKED;
	bool prev_change = value_change_;
	value_change_ = false;
	set_focus(true);
	set_slider_position(event.x);
	if(value_change_) {
		sound::play_UI_sound(game_config::sounds::slider_adjust);
	} else {
		sound::play_UI_sound(game_config::sounds::button_press);
		value_change_ = prev_change;
	}
}

bool slider::requires_event_focus(const SDL_Event* event) const
{
	if(!focus_ || !enabled() || hidden()) {
		return false;
	}
	if(event == NULL) {
		//when event is not specified, signal that focus may be desired later
		return true;
	}

	if(event->type == SDL_KEYDOWN) {
		SDLKey key = event->key.keysym.sym;
		switch(key) {
		case SDLK_LEFT:
		case SDLK_RIGHT:
			return true;
		default:
			break;
		}
	}
	//mouse events are processed regardless of focus
	return false;
}

void slider::handle_event(const SDL_Event& event)
{
	if (!enabled() || hidden())
		return;

	STATE start_state = state_;

	switch(event.type) {
	case SDL_MOUSEBUTTONUP:
		if (!mouse_locked())
			state_ = NORMAL;
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (!mouse_locked())
			mouse_down(event.button);
		break;
	case SDL_MOUSEMOTION:
		if (!mouse_locked())
			mouse_motion(event.motion);
		break;
	case SDL_KEYDOWN:
		if(focus(&event)) {
			const SDL_keysym& key = reinterpret_cast<const SDL_KeyboardEvent&>(event).keysym;
			const int c = key.sym;
			if(c == SDLK_LEFT) {
				sound::play_UI_sound(game_config::sounds::slider_adjust);
				set_value(value_ - increment_);
			} else if(c == SDLK_RIGHT) {
				sound::play_UI_sound(game_config::sounds::slider_adjust);
				set_value(value_ + increment_);
			}
		}
		break;
	default:
		return;
	}
	if (start_state != state_)
		set_dirty(true);
}

template<typename T>
list_slider<T>::list_slider(CVideo &video) :
	slider(video)
{
	set_min(0);
	set_increment(1);
	slider::set_value(0);
}

template<typename T>
list_slider<T>::list_slider(CVideo &video, const std::vector<T> &items) :
	slider(video),
	items_(items)
{
	set_min(0);
	set_increment(1);
	if(items.size() > 0)
	{
		set_max(items.size() - 1);
	}
	slider::set_value(0);
}

template<typename T>
const T& list_slider<T>::item_selected() const
{
	return items_[value()];
}

template<typename T>
bool list_slider<T>::select_item(const T& item)
{
	for(unsigned i = 0, nb = items_.size(); i < nb; ++i)
	{
		if(item == items_[i])
		{
			slider::set_value(i);
			return true;
		}
	}
	return false;
}

template<typename T>
void list_slider<T>::set_items(const std::vector<T> &items)
{
	items_ = items;
	if(items.size() > 0)
	{
		set_max(items.size() - 1);
	}
	slider::set_value(0);
}

// Force compilation of the following template instantiations
template class list_slider< double >;
template class list_slider< int >;
template class list_slider< std::string >;

} //end namespace gui
