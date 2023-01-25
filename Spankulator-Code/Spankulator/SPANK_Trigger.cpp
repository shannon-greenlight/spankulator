#include "Arduino.h"
#include "SPANK_Trigger.h"

SPANK_Trigger::SPANK_Trigger(void)
{
    // Serial.println("Howdy Spank Engine!");
}

float SPANK_Trigger::get_pulse_count(unsigned int rnd)
{
    float rnd_factor = random(-rnd, rnd) / 100.0 * pulse_count;
    float result = pulse_count + rnd_factor;
    // result = max(0, result);
    return result;
}

uint16_t SPANK_Trigger::calc_delay(int rnd)
{
    float delay_adj = rnd * delay / 200.0;
    // reuse rnd here
    rnd = random(int(-delay_adj), int(delay_adj));
    if (delay + rnd < 1)
    {
        return 1;
    }
    return delay + rnd;
}

void SPANK_Trigger::trigger(trig_type type, bool led_on, unsigned long ms)
{
    unsigned int num_pulses = selected_fxn->get_param(NUM_PULSES);
    unsigned int shortest_pulse = selected_fxn->get_param(SHORTEST_PULSE);
    unsigned int longest_pulse = selected_fxn->get_param(LONGEST_PULSE);
    unsigned int randomness = selected_fxn->get_param(RANDOMNESS);
    unsigned int span = longest_pulse - shortest_pulse;
    unsigned int my_delay = calc_delay(0);
    float randomized_pulse_count;
    float delta;

    next_time = ms + my_delay;
    delta = span / (num_pulses - 1);

    if (led_on)
    {
        pulse_count--;
        if (type == TRIG_DOWN)
        {
            delay = my_delay - delta;
        }
        else
        {
            delay = my_delay + delta;
        }
    }

    switch (type)
    {
    case TRIG_UP:
        randomized_pulse_count = led_on ? float(num_pulses - get_pulse_count(randomness) - .5) : float(num_pulses - get_pulse_count(randomness));
        break;
    case TRIG_DOWN:
        randomized_pulse_count = led_on ? float(get_pulse_count(randomness)) : float(get_pulse_count(randomness) - .5);
        break;
    }
    randomized_pulse_count = max(0, randomized_pulse_count);
    aval = randomized_pulse_count / num_pulses * DAC_FS;

    if (pulse_count == 0)
    {
        delay = type == TRIG_UP ? shortest_pulse : longest_pulse;
        pulse_count = num_pulses;
        next_time = millis() + selected_fxn->get_param(INITIAL_DELAY);
        reset_trigger();
    }
}
