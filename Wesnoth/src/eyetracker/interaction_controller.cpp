/*
* Author: Björn
*/
#include "interaction_controller.hpp"
#include "../game_preferences.hpp"
#include "map_location.hpp"
#include "display.hpp"
#include <unistd.h>

#define DWELL_BOUNDARY_X 40
#define DWELL_BOUNDARY_Y 40
using ::std::cerr;

namespace eyetracker
{
SDL_TimerID interaction_controller::timer_id_ = NULL;
gui::widget* interaction_controller::selected_widget_g1_ = NULL;
gui2::twidget* interaction_controller::selected_widget_g2_ = NULL;
gui2::twindow* interaction_controller::selected_window_ = NULL;
map_location* interaction_controller::map_loc_ = NULL;
display* interaction_controller::disp = NULL;
bool right_click_ = false;
bool been_out_of_top_box = true;
int dwell_startX_ = 0;
int dwell_startY_ = 0;

SDL_TimerID interaction_controller::draw_timer_id_ = NULL;
CVideo* interaction_controller::video_ = NULL;
SDL_Rect interaction_controller::previous_rect_ = create_rect(0,0,0,0);
int interaction_controller::remaining_dwell_length_ = 0;
surface interaction_controller::restore_ = NULL;

// REMEMBER: mouse_leave and mouse_enter may not be called one at a time.
// Sometimes there are several calls to mouse_enter in a row or vice versa.
void interaction_controller::mouse_enter(gui::widget* widget, interaction_controller::EVENT_TO_SEND event)
{
//    std::cerr << "Entered GUI1\n";
    if(timer_id_ != NULL)
        stop_timer();
    if(draw_timer_id_ != NULL)
        stop_draw_timer();
    if(selected_widget_g1_ != NULL || selected_widget_g2_ != NULL || selected_window_ != NULL || map_loc_ != NULL)
    {
        reset();
    }
    /*
    When the mouse enters over a widget a timer (or something else, depending on interaction method) will be started.
    When the timer runs out widget.click() will be called, so the widget must implement this method.
    Also, widget.indicate() should produce a visible indicator over it, so that we can see how long time is left.
    */
    selected_widget_g1_ = widget;

    switch (preferences::interaction_method())
    {
    case preferences::DWELL:
        start_timer(event);
        start_draw_timer();
        break;
    case preferences::BLINK:
        // Blink
        break;
    case preferences::SWITCH:
        // Switch
        break;
    }
}


// REMEMBER: mouse_leave and mouse_enter may not be called one at a time.
// Sometimes there are several calls to mouse_enter in a row or vice versa.
void interaction_controller::mouse_enter(gui2::twidget* widget,interaction_controller::EVENT_TO_SEND event)
{
//    std::cerr << "Entered GUI2\n";
    if(timer_id_ != NULL)
        stop_timer();
    if(draw_timer_id_ != NULL)
        stop_draw_timer();
    if(selected_widget_g1_ != NULL || selected_widget_g2_ != NULL || selected_window_ != NULL || map_loc_ != NULL)
    {
        reset();
    }

    selected_widget_g2_ = widget;

    switch (preferences::interaction_method())
    {
    case preferences::DWELL:
        start_timer(event);
        start_draw_timer();
        break;
    case preferences::BLINK:
        // Blink
        break;
    case preferences::SWITCH:
        // Switch
        break;
    }
}
void interaction_controller::mouse_enter(map_location* loc, display* d, interaction_controller::EVENT_TO_SEND event)
{
    if(timer_id_ != NULL)
        stop_timer();
    if(draw_timer_id_ != NULL)
        stop_draw_timer();

    map_loc_ = loc;
    disp = d;

    switch (preferences::interaction_method())
    {
    case preferences::DWELL:
        start_timer(event);
        break;
    case preferences::BLINK:
        // Blink
        break;
    case preferences::SWITCH:
        // Switch
        break;
    }
}

void interaction_controller::mouse_leave(gui::widget *widget) {
    if (widget == selected_widget_g1_)
        mouse_leave_base();
}

void interaction_controller::mouse_leave(gui2::twidget *widget) {
    if (widget == selected_widget_g2_)
        mouse_leave_base();
}




// REMEMBER: mouse_leave and mouse_enter may not be called one at a time.
// Sometimes there are several calls to mouse_enter in a row or vice versa.
void interaction_controller::mouse_leave_base()
{
    switch (preferences::interaction_method())
    {
    case preferences::DWELL:
        if(timer_id_ != NULL)
            stop_timer();
        if(draw_timer_id_ != NULL)
            stop_draw_timer();
        if(restore_ != NULL) {
            restore_background();
            restore_ = NULL;
        }
        break;
    case preferences::BLINK:
        // Blink is selected
        break;
    case preferences::SWITCH:
        // Switch
        break;
    }

    reset();
}
//CheckStillDwelling BOBBY, Veronica, Andreas
void interaction_controller::checkStillDwelling()
{
    if(selected_window_ != NULL){

        int x;
        int y;


        SDL_GetMouseState(&x, &y);
        //cerr<<"STILLDWELL: "<<x<<" "<<y<<"\n";
        if(y<200)//(abs(dwell_startX_ - x) >= DWELL_BOUNDARY_X) || abs(dwell_startY_ - y) >= DWELL_BOUNDARY_Y)
        {
           if(timer_id_ == NULL){
                init_window(selected_window_);
        }
        }else{
            been_out_of_top_box = true;
            stop_timer();
        }
    }
}//End checkSTillDwelling

//Init_window BOBBY Veronica, Andreas
void interaction_controller::init_window(gui2::twindow* window, interaction_controller::EVENT_TO_SEND event)
{
//    std::cerr << "Entered GUI1\n";
    if(timer_id_ != NULL)
        stop_timer();
    if(draw_timer_id_ != NULL)
        stop_draw_timer();
    if(selected_widget_g1_ != NULL || selected_widget_g2_ != NULL || selected_window_ != NULL)
    {
        reset();
    }
    /*
    When the mouse enters over a widget a timer (or something else, depending on interaction method) will be started.
    When the timer runs out widget.click() will be called, so the widget must implement this method.
    Also, widget.indicate() should produce a visible indicator over it, so that we can see how long time is left.
    */
    selected_window_ = window;
    int x;
    int y;
    SDL_GetMouseState(&x, &y);
    bool changedCord = false;
    if(x!=dwell_startX_ || y !=dwell_startY_){
        changedCord = true;
        dwell_startX_ = x;
        dwell_startY_ = y;
    }
    switch (preferences::interaction_method())
    {
    case preferences::DWELL:
        if(changedCord && been_out_of_top_box){
            been_out_of_top_box = false;
            start_timer(event);
        }
        break;
    case preferences::BLINK:
        // Blink
        break;
    case preferences::SWITCH:
        // Switch
        break;
    }
}//End init windoe
void interaction_controller::click(int mousex, int mousey, Uint8 mousebutton)
{
    SDL_Event fake_event;
    fake_event.type = SDL_MOUSEBUTTONDOWN;
    fake_event.button.button = mousebutton;
    fake_event.button.type = SDL_MOUSEBUTTONDOWN;
    fake_event.button.state = SDL_PRESSED;
    fake_event.button.which = 0;
    fake_event.button.x = mousex;
    fake_event.button.y = mousey;
    SDL_PushEvent(&fake_event);

    fake_event.button.state = SDL_RELEASED;
    fake_event.button.type = SDL_MOUSEBUTTONUP;
    fake_event.type=SDL_MOUSEBUTTONUP;
    fake_event.button.type=SDL_MOUSEBUTTONUP;
    SDL_PushEvent(&fake_event);
}
void interaction_controller::right_or_left_click(int x,int y){
    if(right_click_ && map_loc_ != NULL){
        click(x,y,SDL_BUTTON_RIGHT);
        right_click_ = false;
    }
    else{
        click(x,y);
    }
}

void interaction_controller::double_click(int mousex, int mousey)
{
    click(mousex, mousey);
    click(mousex, mousey);
}

void interaction_controller::reset()
{
    if(restore_ != NULL) {
        restore_background();
        restore_ = NULL;
    }
    selected_widget_g1_ = NULL;
    selected_widget_g2_ = NULL;
    selected_window_ = NULL;
    map_loc_ = NULL;
}

Uint32 interaction_controller::callback(Uint32 interval, void* param)
{
    (void)interval;
    int tmp = (int) param;
    int x,y;
    interaction_controller::EVENT_TO_SEND event = (interaction_controller::EVENT_TO_SEND) tmp;
    if(selected_widget_g1_ != NULL)
    {
        //Potentiell Johan konflikt (Andreas & Christoffer)
        //SDL_Rect rect = selected_widget_g1_->location();
        //x = rect.x + rect.w/2;
        //y = rect.y + rect.h/2;
        SDL_GetMouseState(&x,&y);
    }
    else if(selected_widget_g2_ != NULL)
    {
        //Potentiell Johan konflikt (Andreas & Christoffer)
        //x = selected_widget_g2_->get_x() + selected_widget_g2_->get_width()/2;
        //y = selected_widget_g2_->get_y() + selected_widget_g2_->get_height()/2;
        SDL_GetMouseState(&x,&y);
    }
    else if(map_loc_ != NULL){
        SDL_GetMouseState(&x,&y);
    }
    else if(selected_window_ != NULL){
        x = dwell_startX_;
        y = dwell_startY_;
    }
    else
    {
        throw "InteractionController: Trying to click a widget but no widget has been selected.";
    }

    if(event == CLICK)
    {
        right_or_left_click(x,y);
    }
    else if(event == DOUBLE_CLICK)
    {
        double_click(x,y);
    }
    else if(event == REPEATING_CLICK)
    {
        click(x,y);
        return interval; // returning interval to next click
    }

    reset();
    return 0;
}


void interaction_controller::press_switch(){
    if (preferences::interaction_method() == preferences::SWITCH) {
        int x,y;
        SDL_GetMouseState(&x,&y);
        right_or_left_click(x,y);
    }
}


// Functions like callback but with limited functionality.
// Should be used to simulate a MouseClick event when a blink
// occurs.
//
// x : X-position of where the click event should occur.
// y : Y-position of where the click event should occur.
//
// Author: Robert, Christoffer, Andreas, Björn, Johan
// Version: 28-03-2013
void interaction_controller::blink(int x,int y){
    if(preferences::interaction_method() == preferences::BLINK){
        //int x,y;

        if(selected_window_ != NULL)
        {
            x = dwell_startX_;
            y = dwell_startY_;
        }
        else if(map_loc_ == NULL && selected_widget_g2_ == NULL && selected_widget_g1_ == NULL)
        {
           return;
        }

        right_or_left_click(x,y);

        reset();
    }
    return;
}



void interaction_controller::toggle_right_click(bool value){
    right_click_ = value;
}

bool interaction_controller::get_right_click(){
    return right_click_;
}

void interaction_controller::start_timer(interaction_controller::EVENT_TO_SEND event)
{
    if(timer_id_ == NULL && (selected_widget_g1_ != NULL || selected_widget_g2_ != NULL || map_loc_ != NULL || selected_window_ != NULL))
    {
        timer_id_ = SDL_AddTimer(preferences::gaze_length(), callback, (void*) event);
    }
    else
    {
        throw "Trying to start timer without stopping last timer or without selected widget";
    }
}
void interaction_controller::start_draw_timer() {
    if(draw_timer_id_ == NULL && (selected_widget_g1_ != NULL || selected_widget_g2_ != NULL || map_loc_ != NULL || selected_window_ != NULL))
    {
        remaining_dwell_length_ = preferences::gaze_length();
        draw_timer_id_ = SDL_AddTimer(75, draw_callback, NULL);
    }
    else
    {
        throw "Trying to start draw timer without stopping last timer or without selected widget";
    }
}
void interaction_controller::stop_timer()
{
    SDL_RemoveTimer(timer_id_);
    timer_id_ = NULL;
}
void interaction_controller::stop_draw_timer()
{
    SDL_RemoveTimer(draw_timer_id_);
    draw_timer_id_ = NULL;
}

void interaction_controller::set_indicator_display(CVideo* video)
{
    video_ = video;
}
Uint32 interaction_controller::draw_callback(Uint32 interval, void* param)
{
    remaining_dwell_length_ -= interval;
    if(remaining_dwell_length_ <= 0) {
        return 0;
    }
    double size_multiplier = (double)remaining_dwell_length_ / (double)preferences::gaze_length();
    SDL_Rect indicator_rect;
    if(selected_widget_g1_ != NULL)
    {
        indicator_rect = selected_widget_g1_->indicator_rect();
    }
    else if(selected_widget_g2_ != NULL)
    {
        indicator_rect = selected_widget_g2_->indicator_rect();
    }
    else if(map_loc_ != NULL){
        //SDL_GetMouseState(&x,&y);
        return interval;
    }
    else if(selected_window_ != NULL){
        //x = dwell_startX_;
        //y = dwell_startY_;
        return interval;
    }
    else
    {
        throw "InteractionController: Trying to click a widget but no widget has been selected.";
    }

    draw_indicator(indicator_rect,size_multiplier);
    return interval;
}
void interaction_controller::restore_background()
{
    if(restore_ != NULL) {
        surface& surf = video_->getSurface();
        sdl_blit(restore_,NULL,surf,&previous_rect_);
        SDL_Flip(surf);
    }
}
void interaction_controller::draw_indicator(SDL_Rect indicator_rect, double size_multiplier)
{
    int max_radius = indicator_rect.h / 2;
	int radius = (int) (max_radius * size_multiplier);
    surface& surf = video_->getSurface();
	surface ind = create_neutral_surface(2 * radius, 2 * radius);

    if(restore_ == NULL) {
        restore_ = create_neutral_surface(indicator_rect.w, indicator_rect.h);
        sdl_blit(surf,&indicator_rect,restore_,NULL);
    }
    else {
        restore_background();
        sdl_blit(surf,&indicator_rect,restore_,NULL);
    }
    previous_rect_ = indicator_rect;

	Uint32 pixel = SDL_MapRGBA(ind->format, 0, 254, 0, 60);

	double r = (double) radius;
	ptrdiff_t start = reinterpret_cast<ptrdiff_t>(ind->pixels);
	unsigned w = ind->w;
    int cy = indicator_rect.y + indicator_rect.h/2;
    int cx = indicator_rect.x + indicator_rect.w/2;
	for (int y = 0; y < 2 * radius; y++)
	{
		double dy = abs((cy - radius + y) - cy);

		for(int x = 0; x < 2 * radius; x++)
		{
			double dx = abs((cx - radius + x) - cx);
			double dist = sqrt(dx * dx + dy * dy);
			if(dist < r)
				*reinterpret_cast<Uint32*>(start + (y * w * 4) + x * 4) = pixel;
		}
	}

	SDL_Rect target = create_rect(cx - radius, cy - radius, radius * 2, radius * 2);

	sdl_blit(ind,NULL,surf,&target);
	SDL_Flip(surf);
}
}
