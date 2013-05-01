/*
* Author: Björn
*/
#include "interaction_controller.hpp"
#include "../game_preferences.hpp"
#include "map_location.hpp"
#include "display.hpp"
#include "video.hpp"
#include <unistd.h>

#define DWELL_BOUNDARY_X 40
#define DWELL_BOUNDARY_Y 40
#define DIALOG_INDICATOR_HEIGHT 200
#define DIALOG_INDICATOR_STARTY 30
#define DIALOG_INDICATOR_WIDTH_OFFSET 140
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
SDL_Rect interaction_controller::indicator_rect_ = create_rect(0,0,0,0);
SDL_Rect interaction_controller::dialog_rect_ = create_rect(0,0,0,0);
int interaction_controller::remaining_slices_ = 4;
surface interaction_controller::restore_ = NULL;
surface interaction_controller::restore_dialog_ = NULL;
surface current_surface = NULL;
bool draw_indicator_ = false;
bool show_dialog_indicator_ = false;
interaction_controller::INDICATOR_TYPE indicator_type_ = interaction_controller::CLOCK;

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

    indicator_rect_ = selected_widget_g1_->indicator_rect();
    draw_indicator_ = true;

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

    indicator_rect_ = selected_widget_g2_->indicator_rect();
    draw_indicator_ = true;

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

    if(selected_widget_g1_ != NULL || selected_widget_g2_ != NULL || selected_window_ != NULL || map_loc_ != NULL)
    {
        reset();
    }

    map_loc_ = loc;
    disp = d;

    indicator_rect_ = disp->indicator_rect();
    draw_indicator_ = true;

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

void interaction_controller::mouse_leave(gui::widget *widget)
{
    if (widget == selected_widget_g1_)
        mouse_leave_base();
}

void interaction_controller::mouse_leave(gui2::twidget *widget)
{
    if (widget == selected_widget_g2_)
        mouse_leave_base();
}

void interaction_controller::mouse_leave(map_location* loc, display* d)
{
    if (loc == map_loc_)
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
    if(selected_window_ != NULL)
    {

        int x;
        int y;


        SDL_GetMouseState(&x, &y);
        std::pair<int,int> res = preferences::resolution();
        //cerr<<"STILLDWELL: "<<x<<" "<<y<<"\n";
        if(y<DIALOG_INDICATOR_HEIGHT && y > DIALOG_INDICATOR_STARTY && x < res.first - DIALOG_INDICATOR_WIDTH_OFFSET)//(abs(dwell_startX_ - x) >= DWELL_BOUNDARY_X) || abs(dwell_startY_ - y) >= DWELL_BOUNDARY_Y)
        {
            if(timer_id_ == NULL)
            {
                init_window(selected_window_);
            }
        }
        else
        {
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
    if(x!=dwell_startX_ || y !=dwell_startY_)
    {
        changedCord = true;
        dwell_startX_ = x;
        dwell_startY_ = y;
    }
    switch (preferences::interaction_method())
    {
    case preferences::DWELL:
        show_dialog_indicator_ = true;
        if(changedCord && been_out_of_top_box)
        {
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

void interaction_controller::toggle_dialog_indicator()
{
    if(current_surface == NULL || selected_window_ == NULL) return;
    if(show_dialog_indicator_)
    {
        surface draw_surface = create_neutral_surface(dialog_rect_.w,dialog_rect_.h);
        unsigned w = draw_surface->w;
        Uint32 pixel = SDL_MapRGBA(draw_surface->format, 254, 0, 0, 60);
        ptrdiff_t start = reinterpret_cast<ptrdiff_t>(draw_surface->pixels);
        for(int x = dialog_rect_.x; x < dialog_rect_.w; x++)
        {
            for(int y = dialog_rect_.y; y < dialog_rect_.h; y++)
            {
                *reinterpret_cast<Uint32*>(start + (y * w * 4) + x * 4) = pixel;
            }
        }
        sdl_blit(draw_surface,NULL,current_surface,&dialog_rect_);
        update_rect(dialog_rect_);
    }
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

    fake_event.button.state = SDL_RELEASED;
    fake_event.button.type = SDL_MOUSEBUTTONUP;
    fake_event.type=SDL_MOUSEBUTTONUP;
    fake_event.button.type=SDL_MOUSEBUTTONUP;
    SDL_PushEvent(&fake_event);
    if(draw_timer_id_ != NULL)
    {
        stop_draw_timer();
    }
}
void interaction_controller::right_or_left_click(int x,int y)
{
    if(right_click_ && map_loc_ != NULL)
    {
        click(x,y,SDL_BUTTON_RIGHT);
        right_click_ = false;
    }
    else
    {
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

    draw_indicator_ = false;
    show_dialog_indicator_ = false;
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
        SDL_GetMouseState(&x,&y);
    }
    else if(selected_widget_g2_ != NULL)
    {
        SDL_GetMouseState(&x,&y);
    }
    else if(map_loc_ != NULL)
    {
        SDL_GetMouseState(&x,&y);
    }
    else if(selected_window_ != NULL)
    {
        x = dwell_startX_;
        y = dwell_startY_;
    }
    else
    {
        std::cerr << "Trying to click a widget that does not exist";
        stop_timer();
        return 0;
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
        stop_draw_timer();
        start_draw_timer();
        return interval; // returning interval to next click
    }

    reset();
    return 0;
}


void interaction_controller::press_switch()
{
    if (preferences::interaction_method() == preferences::SWITCH)
    {
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
void interaction_controller::blink(int x,int y)
{
    if(preferences::interaction_method() == preferences::BLINK)
    {
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



void interaction_controller::toggle_right_click(bool value)
{
    right_click_ = value;
}

bool interaction_controller::get_right_click()
{
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
        std::cerr << "Trying to start new timer without stopping previous timer or selected widget was null";
        if(timer_id_ != NULL)
        {
            stop_timer();
        }
    }
}
void interaction_controller::start_draw_timer()
{
    if(draw_timer_id_ == NULL && (selected_widget_g1_ != NULL || selected_widget_g2_ != NULL || map_loc_ != NULL || selected_window_ != NULL))
    {
        remaining_slices_ = 4;
        draw_timer_id_ = SDL_AddTimer(preferences::gaze_length()/4, draw_callback, NULL);
    }
    else
    {
        std::cerr << "Trying to start new draw timer without stopping previous timer or selected widget was null";
        if(draw_timer_id_ != NULL)
        {
            stop_draw_timer();
        }
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

Uint32 interaction_controller::draw_callback(Uint32 interval, void* param)
{
    remaining_slices_--;
    if(remaining_slices_ <= 0)
    {
        stop_draw_timer();
        return 0;
    }
    return interval;
}

void interaction_controller::restore_background()
{
    if(restore_ != NULL)
    {
        SDL_Rect tempDestRect = {indicator_rect_.x,indicator_rect_.y,indicator_rect_.w,indicator_rect_.h};
        sdl_blit(restore_,NULL,current_surface,&tempDestRect);
        update_rect(indicator_rect_);
        restore_ = NULL;
    }
    else if(restore_dialog_ != NULL)
    {
        sdl_blit(restore_dialog_,NULL,current_surface,&dialog_rect_);
        update_rect(dialog_rect_);
        restore_dialog_ = NULL;
    }
    else {
        //std::cerr << "restore_background called even though no background has been stored\n";
    }
}

//BOBBY OLD INDICATOR!
//void interaction_controller::draw_indicator(surface surf)
//{
//    if(draw_timer_id_ != NULL){
//        double size_multiplier = (double)remaining_dwell_length_ / (double)preferences::gaze_length();
//        int max_radius = indicator_rect_.h / 2;
//        int radius = (int) (max_radius * size_multiplier);
//        surface ind = create_neutral_surface(2 * radius, 2 * radius);
//
//        if(restore_ == NULL) {
//            restore_ = create_neutral_surface(indicator_rect_.w, indicator_rect_.h);
//        }
//        sdl_blit(surf,&indicator_rect_,restore_,NULL);
//        Uint32 pixel = SDL_MapRGBA(ind->format, 254, 0, 0, 60);
//
//        double r = (double) radius;
//        ptrdiff_t start = reinterpret_cast<ptrdiff_t>(ind->pixels);
//        unsigned w = ind->w;
//        int cy = indicator_rect_.y + indicator_rect_.h/2;
//        int cx = indicator_rect_.x + indicator_rect_.w/2;
//        for (int y = 0; y < 2 * radius; y++)
//        {
//            double dy = abs((cy - radius + y) - cy);
//
//            for(int x = 0; x < 2 * radius; x++)
//            {
//                double dx = abs((cx - radius + x) - cx);
//                double dist = sqrt(dx * dx + dy * dy);
//                if(dist < r)
//                    *reinterpret_cast<Uint32*>(start + (y * w * 4) + x * 4) = pixel;
//            }
//        }
//
//        SDL_Rect target = create_rect(cx - radius, cy - radius, radius * 2, radius * 2);
//
//        sdl_blit(ind,NULL,surf,&target);
//        update_rect(indicator_rect_);
//    }
//    current_surface = surf;
//}

// Sets the surface used to restore the image between every draw of the indicator
//
// surf : surface object to blit for the restore
//
// Author: Christoffer, Johan
// Version: 29-04-2013
void interaction_controller::set_indicator_restore_surface(surface surf)
{
    if(draw_indicator_)
    {
        if(restore_ == NULL)
        {
            restore_ = create_neutral_surface(indicator_rect_.w, indicator_rect_.h);
        }
        sdl_blit(surf,&indicator_rect_,restore_,NULL);
    }
    else if(show_dialog_indicator_)
    {
        //We reset this rect every time since the resolution might have changed
        std::pair<int,int> res = preferences::resolution();
        dialog_rect_ = {0,DIALOG_INDICATOR_STARTY,res.first - DIALOG_INDICATOR_WIDTH_OFFSET,DIALOG_INDICATOR_HEIGHT};
        if(restore_dialog_ == NULL)
        {
            restore_dialog_ = create_neutral_surface(res.first - DIALOG_INDICATOR_WIDTH_OFFSET,DIALOG_INDICATOR_HEIGHT);
        }
        sdl_blit(surf,&dialog_rect_,restore_dialog_,NULL);
    }
}

//Draw indicator using Tårtenham's circle algorithm
// NOTE: Remember to call set_indicator_restore_surface before drawing indicator!
void interaction_controller::draw_indicator(surface surf)
{
    if(draw_indicator_ && remaining_slices_ > 0)
    {
        int radius = indicator_rect_.h / 2;
        surface ind = create_neutral_surface(2 * radius, 2 * radius);

        Uint32 pixel = SDL_MapRGBA(ind->format, 254, 254, 254, 60);

        double r = (double) radius;
        ptrdiff_t start = reinterpret_cast<ptrdiff_t>(ind->pixels);
        unsigned w = ind->w;
        int cy = indicator_rect_.y + indicator_rect_.h/2;
        int cx = indicator_rect_.x + indicator_rect_.w/2;

        switch(indicator_type_)
        {
        //Clock type indicator
        case interaction_controller::CLOCK:
            for (int y = 0; y < 2 * radius; y++)
            {
                double dy = abs(y - radius);

                for(int x = 0; x < 2 * radius; x++)
                {
                    double dx = abs(x - radius);
                    double dist = sqrt(dx * dx + dy * dy);
                    if(dist < r)
                    {
                        if(preferences::interaction_method() == preferences::DWELL){
                            switch(remaining_slices_)
                            {
                            case 4:
                                *reinterpret_cast<Uint32*>(start + (y * w * 4) + x * 4) = pixel;
                                break;
                            case 3:
                                if(!(x > radius && y < radius)) *reinterpret_cast<Uint32*>(start + (y * w * 4) + x * 4) = pixel;
                                break;
                            case 2:
                                if(x < radius) *reinterpret_cast<Uint32*>(start + (y * w * 4) + x * 4) = pixel;
                                break;
                            case 1:
                                if(x < radius && y < radius) *reinterpret_cast<Uint32*>(start + (y * w * 4) + x * 4) = pixel;
                                break;
                            }
                        }
                        else{
                            *reinterpret_cast<Uint32*>(start + (y * w * 4) + x * 4) = pixel;
                        }
                    }
                }
            }
            break;
        //Shrinking indicator
        case interaction_controller::ZOOM:
        double size_multiplier = remaining_slices_ / 4.0;
        int draw_radius = (int) (radius * size_multiplier);
            for (int y = 0; y < 2 * radius; y++)
            {
                double dy = abs((cy - radius + y) - cy);

                for(int x = 0; x < 2 * radius; x++)
                {
                    double dx = abs((cx - radius + x) - cx);
                    double dist = sqrt(dx * dx + dy * dy);
                    if(dist < draw_radius)
                        *reinterpret_cast<Uint32*>(start + (y * w * 4) + x * 4) = pixel;
                }
            }
            break;
        }

        SDL_Rect target = create_rect(cx - radius, cy - radius, radius * 2, radius * 2);

        sdl_blit(ind,NULL,surf,&target);
        update_rect(indicator_rect_);
    }
    current_surface = surf;
}

//Overrides the indicator rect with input rect
void interaction_controller::override_indiactor_rect(SDL_Rect rect)
{
    indicator_rect_ = rect;
}

//Overrides the indicator type with inpute type
void interaction_controller::override_indicator_type(interaction_controller::INDICATOR_TYPE type)
{
    indicator_type_ = type;
}

}
