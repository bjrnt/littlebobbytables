
#ifndef GUI_WIDGETS_FAKEMINIMAP_HPP_INCLUDED
#define GUI_WIDGETS_FAKEMINIMAP_HPP_INCLUDED

#include "gui/widgets/widget.hpp"

namespace gui2{

class fake_minimap : public gui2::twidget {
public:
    void set_indicator_rect(const SDL_Rect &);
    SDL_Rect indicator_rect();
    virtual bool disable_click_dismiss() const {return false;}
    virtual gui2::iterator::twalker_* create_walker(){return NULL;}
    virtual void request_reduce_width(const unsigned maximum_width) {}
private:
    SDL_Rect indicator_rect_;
    virtual gui2::tpoint calculate_best_size() const {}
};

}

#endif // GUI_WIDGETS_FAKEMINIMAP_HPP_INCLUDED
