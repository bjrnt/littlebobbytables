/*
* Author: Björn
*/
#include "../widgets/widget.hpp"
#include "../gui/widgets/widget.hpp"
#include "map_location.hpp"
#include "display.hpp"

namespace eyetracker
{
class interaction_controller
{

public:
    enum INTERACTION_METHOD { DWELL, BLINK, SWITCH };
    enum EVENT_TO_SEND {CLICK, DOUBLE_CLICK, REPEATING_CLICK, RIGHT_CLICK};

private:
    static INTERACTION_METHOD interaction_method_; // Currently selected interaction method
    static SDL_TimerID timer_id_;
    static gui::widget* selected_widget_g1_;
    static gui2::twidget* selected_widget_g2_;
    static map_location* map_loc_;
    static display* disp;

    static void click(int mousex, int mousey, Uint8 mousebutton = SDL_BUTTON_LEFT);
    static void double_click(int mousex, int mousey);

    static Uint32 callback(Uint32 interval, void* widget); // Called after a timer has expired
    static void stop_timer();
    static void reset(); // resets timer and internal state
    static void start_timer(interaction_controller::EVENT_TO_SEND event); // Sets the selected widget and timer id, starts a timer

public:
    static void set_interaction_method(INTERACTION_METHOD interaction_method); // Used to change interaction method

    static void init();

    static void blink();

    static void mouse_enter(gui::widget* widget, interaction_controller::EVENT_TO_SEND event = CLICK); // Should be called by GUI1 widgets when the mouse enters over it
    static void mouse_enter(gui2::twidget* widget, interaction_controller::EVENT_TO_SEND event = CLICK); // Should be called by GUI2 widgets when the mouse enters over it
    static void mouse_enter(map_location* loc, display* d, interaction_controller::EVENT_TO_SEND event = CLICK); //Should be called by display when hex is selected
    static void mouse_leave();
};
}
