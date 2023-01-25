#ifndef SPANK_ENGINE_h
#define SPANK_ENGINE_h
#include <Greenface_gadget.h>
#include "hardware_defs.h"

typedef enum _trig_type
{
    TRIG_UP = 0,
    TRIG_DOWN,
} trig_type;

extern Greenface_gadget *selected_fxn;
extern void reset_trigger();

class SPANK_Trigger
{
public:
    SPANK_Trigger(void);
    bool triggered;
    unsigned long next_time;
    float delay;
    int pulse_count;
    uint16_t aval;

    void trigger(trig_type type, bool led_on, unsigned long ms);
    uint16_t calc_delay(int rnd);

private:
    float get_pulse_count(unsigned int rnd);
};

#endif
