/*
* Author: Björn
*/
#include "interaction_controller.hpp"
#include "../game_preferences.hpp"

namespace eyetracker
{
    interaction_controller::INTERACTION_METHOD interaction_controller::interaction_method_ = DWELL;
    SDL_TimerID interaction_controller::timer_id_ = NULL;
    gui::widget* interaction_controller::selected_widget_g1_ = NULL;
    gui2::twidget* interaction_controller::selected_widget_g2_ = NULL;

void interaction_controller::set_interaction_method(interaction_controller::INTERACTION_METHOD interaction_method)
{
    interaction_method_ = interaction_method;
}
void interaction_controller::mouse_enter(gui::widget* widget)
{
    /*
    When the mouse enters over a widget a timer (or something else, depending on interaction method) will be started.
    When the timer runs out widget.click() will be called, so the widget must implement this method.
    Also, widget.indicate() should produce a visible indicator over it, so that we can see how long time is left.
    */
    selected_widget_g1_ = widget;

    switch (interaction_method_)
    {
    case interaction_controller::DWELL:
        start_timer();
        break;
    case interaction_controller::BLINK:
        // Blink
        break;
    case interaction_controller::SWITCH:
        // Switch
        break;
    }
}
void interaction_controller::mouse_enter(gui2::twidget* widget)
{
    selected_widget_g2_ = widget;

    switch (interaction_method_)
    {
    case interaction_controller::DWELL:
        start_timer();
        break;
    case interaction_controller::BLINK:
        // Blink
        break;
    case interaction_controller::SWITCH:
        // Switch
        break;
    }
}
void interaction_controller::mouse_leave(gui::widget* widget)
{
    if(widget != selected_widget_g1_) {
        return;
    }

    /*
    This method is called when the mouse leaves a widget, so that the timer and indicator is removed from the previously selected widget.
    */
    switch (interaction_method_)
    {
    case interaction_controller::BLINK:
        // Blink is selected
        break;
    case interaction_controller::DWELL:
        stop_timer();
        break;
    case interaction_controller::SWITCH:
        // Switch
        break;
    }

    selected_widget_g1_ = NULL;
}
void interaction_controller::mouse_leave(gui2::twidget* widget)
{
    if(widget != selected_widget_g2_) {
        return;
    }

    switch (interaction_method_)
    {
    case interaction_controller::BLINK:
        // Blink is selected
        break;
    case interaction_controller::DWELL:
        stop_timer();
        break;
    case interaction_controller::SWITCH:
        // Switch
        break;
    }

    selected_widget_g2_ = NULL;
}
void click(int mousex, int mousey) {
    SDL_Event fake_event;
    fake_event.type = SDL_MOUSEBUTTONDOWN;
    fake_event.button.button = SDL_BUTTON_LEFT;
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
Uint32 interaction_controller::callback(Uint32 interval, void* param)
{
    //selected_widget_.indicate();
    if(selected_widget_g1_ != NULL) {
        SDL_Rect rect = selected_widget_g1_->location();
        click(rect.x + rect.w/2, rect.y + rect.h/2);
    }
    else if(selected_widget_g2_ != NULL) {
        click(selected_widget_g2_->get_x() + selected_widget_g2_->get_width()/2, selected_widget_g2_->get_y() + selected_widget_g2_->get_height()/2);
    }

    stop_timer();
    return 0;
}
void interaction_controller::start_timer()
{
    if(timer_id_ == NULL) {
        if(selected_widget_g1_ != NULL || selected_widget_g2_ != NULL) {
            timer_id_ = SDL_AddTimer(preferences::gaze_length(), callback, 0);
        }
    }
}
void interaction_controller::stop_timer()
{
    if(timer_id_ != NULL)
    {
        SDL_RemoveTimer(timer_id_);
        timer_id_ = NULL;
        if(selected_widget_g1_ != NULL) {
            selected_widget_g1_ = NULL;
        }
        else if(selected_widget_g2_ != NULL) {
            selected_widget_g2_ = NULL;
        }
    }
}
}
