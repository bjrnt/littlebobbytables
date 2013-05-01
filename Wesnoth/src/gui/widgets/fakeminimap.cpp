
#include "gui/widgets/fakeminimap.hpp"
#include "display.hpp"


namespace gui2{
    void fake_minimap::set_indicator_rect(const SDL_Rect & maparea){
        indicator_rect_.x = maparea.x;
        indicator_rect_.y = maparea.y;
        indicator_rect_.w = maparea.w;
        indicator_rect_.h = maparea.w;
    }

    SDL_Rect fake_minimap::indicator_rect(){
        return indicator_rect_;
    }
}
