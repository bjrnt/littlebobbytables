/* $Id: save_blocker.cpp 52533 2012-01-07 02:35:17Z shadowmaster $ */
/*
   Copyright (C) 2009 - 2012 by Daniel Franke.
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "save_blocker.hpp"

play_controller* save_blocker::controller_ = NULL;
void (play_controller::*save_blocker::callback_)() = NULL;
SDL_sem* save_blocker::sem_ = SDL_CreateSemaphore(1);

save_blocker::save_blocker() {
	block();
}

save_blocker::~save_blocker() {
	unblock();
	if(controller_ && callback_) {
		(controller_->*callback_)();
		controller_ = NULL;
		callback_ = NULL;
	}
}

void save_blocker::on_unblock(play_controller* controller, void (play_controller::*callback)()) {
	if(try_block()) {
		unblock();
		(controller->*callback)();
	} else {
		controller_ = controller;
		callback_ = callback;
	}
}

bool save_blocker::saves_are_blocked() {
	return SDL_SemValue(sem_) == 0;
}

void save_blocker::block() {
	SDL_SemWait(sem_);
}

bool save_blocker::try_block() {
	return SDL_SemTryWait(sem_) == 0;
}

void save_blocker::unblock() {
	assert(SDL_SemValue(sem_) == 0);
	SDL_SemPost(sem_);
}
