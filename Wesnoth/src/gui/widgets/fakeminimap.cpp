
#include "gui/widgets/fakeminimap.hpp"
#include "display.hpp"


namespace gui2{
    void fake_minimap::set_indicator_rect(int x, int y, int w, int h){
        indicator_rect_.x = x;
        indicator_rect_.y = y;
        indicator_rect_.w = w;
        indicator_rect_.h = w;
    }

    SDL_Rect fake_minimap::indicator_rect(){
        return indicator_rect_;
    }
}
