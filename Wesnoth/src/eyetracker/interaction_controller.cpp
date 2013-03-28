/*
* Author: Björn
*/
#include "interaction_controller.hpp"
#include "../game_preferences.hpp"
#include "map_location.hpp"
#include "display.hpp"

namespace eyetracker
{
SDL_TimerID interaction_controller::timer_id_ = NULL;
gui::widget* interaction_controller::selected_widget_g1_ = NULL;
gui2::twidget* interaction_controller::selected_widget_g2_ = NULL;
map_location* interaction_controller::map_loc_ = NULL;
display* interaction_controller::disp = NULL;
bool right_click_ = false;

// REMEMBER: mouse_leave and mouse_enter may not be called one at a time.
// Sometimes there are several calls to mouse_enter in a row or vice versa.
void interaction_controller::mouse_enter(gui::widget* widget, interaction_controller::EVENT_TO_SEND event)
{
//    std::cerr << "Entered GUI1\n";
    if(timer_id_ != NULL)
        stop_timer();
    if(selected_widget_g1_ != NULL || selected_widget_g2_ != NULL)
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
    if(selected_widget_g1_ != NULL || selected_widget_g2_ != NULL)
    {
        reset();
    }

    selected_widget_g2_ = widget;

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
void interaction_controller::mouse_enter(map_location* loc, display* d, interaction_controller::EVENT_TO_SEND event)
{
    if(timer_id_ != NULL)
        stop_timer();

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

// REMEMBER: mouse_leave and mouse_enter may not be called one at a time.
// Sometimes there are several calls to mouse_enter in a row or vice versa.
void interaction_controller::mouse_leave()
{
/*    if(selected_widget_g1_ != NULL)
        std::cerr << "Left GUI1\n";
    else if(selected_widget_g2_ != NULL)
        std::cerr << "Left GUI2\n";
*/
    switch (preferences::interaction_method())
    {
    case preferences::DWELL:
        if(timer_id_ != NULL)
            stop_timer();
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
    fake_event.type=SDL_MOUSEBUTTONUP;
    fake_event.button.type=SDL_MOUSEBUTTONUP;
    SDL_PushEvent(&fake_event);
}
void interaction_controller::double_click(int mousex, int mousey)
{
    click(mousex, mousey);
    click(mousex, mousey);
}

void interaction_controller::reset()
{
    selected_widget_g1_ = NULL;
    selected_widget_g2_ = NULL;
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
    else
    {
        throw "InteractionController: Trying to click a widget but no widget has been selected.";
    }

    if(event == CLICK)
    {
        if(right_click_){
            click(x,y,SDL_BUTTON_RIGHT);
            right_click_ = false;
        }
        else{
            click(x,y);
        }
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

    mouse_leave();
    return 0;
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

        if(selected_widget_g1_ != NULL)
        {
            //SDL_Rect rect = selected_widget_g1_->location();
            //x = rect.x + rect.w/2;
            //y = rect.y + rect.h/2;
        }
        else if(selected_widget_g2_ != NULL)
        {
          //  x = selected_widget_g2_->get_x() + selected_widget_g2_->get_width()/2;
          //  y = selected_widget_g2_->get_y() + selected_widget_g2_->get_height()/2;
        }
        else if(map_loc_ != NULL)
        {
           // SDL_GetMouseState(&x,&y);
        }
        else
        {
            return;
        }

        if(right_click_){
            click(x,y,SDL_BUTTON_RIGHT);
            right_click_ = false;
        }
        else{
            click(x,y);
        }

        mouse_leave();
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
    if(timer_id_ == NULL && (selected_widget_g1_ != NULL || selected_widget_g2_ != NULL || map_loc_ != NULL))
    {
        timer_id_ = SDL_AddTimer(preferences::gaze_length(), callback, (void*) event);
    }
    else
    {
        throw "Trying to start timer without stopping last timer or without selected widget";
    }
}
void interaction_controller::stop_timer()
{
    SDL_RemoveTimer(timer_id_);
    timer_id_ = NULL;
}
}
