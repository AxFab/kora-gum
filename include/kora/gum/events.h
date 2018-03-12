#ifndef _KORA_GUM_EVENTS_H
#define _KORA_GUM_EVENTS_H 1

#include <kora/gum/core.h>

GUM_event_manager *gum_event_manager(GUM_cell *root, GUM_surface *win);
void gum_event_loop(GUM_event_manager *evm);


struct GUM_event
{
    int type;
    int param0;
    int param1;
};

enum {
    GUM_EV_DESTROY,
    GUM_EV_EXPOSE,
    GUM_EV_KEY_PRESS,
    GUM_EV_KEY_RELEASE,
    GUM_EV_MOTION,
    GUM_EV_BTN_PRESS,
    GUM_EV_BTN_RELEASE,

    GUM_EV_OUT,
    GUM_EV_OVER,
    GUM_EV_DOWN,
    GUM_EV_UP,
    GUM_EV_FOCUS,
    GUM_EV_FOCUS_OUT,

    GUM_EV_CLICK,
    GUM_EV_DOUBLECLICK,
    GUM_EV_TRIPLECLICK,
    GUM_EV_RIGHTCLICK,

    GUM_EV_PREVIOUS,
    GUM_EV_NEXT,
    GUM_EV_WHEEL_UP,
    GUM_EV_WHEEL_DOWN,
    GUM_EV_WHEEL_CLICK,
};


#endif  /* _KORA_GUM_EVENTS_H */
