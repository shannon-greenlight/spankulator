uint16_t _stretch_params[6];
uint16_t _stretch_mins[] = {1, 1, 1, 0, 0, 0};
uint16_t _stretch_maxs[] = {9999, 9999, 99, DAC_FS, DAC_FS, 9999};
uint16_t _stretch_init_vals[] = {10, 100, 1, DAC_FS, 512, 1000};
uint16_t *stretch_stuff[] = {_stretch_params, _stretch_mins, _stretch_maxs, _stretch_init_vals};
String stretch_labels[] = {"On Time: ", "Off time: ", "#Pulses: ", "Level 1: ", "Level 2: ", "Init Delay: "};
Greenface_gadget stretch_spanker("Stretch", stretch_labels, stretch_stuff, sizeof(_stretch_params) / sizeof(_stretch_params[0]));

enum
{
  STRETCH_ON_TIME = 0,
  STRETCH_OFF_TIME,
  STRETCH_NUM_PULSES,
  STRETCH_LVL1,
  STRETCH_LVL2,
  STRETCH_INIT_DELAY,
};

void stretch_trigger()
{
  static bool led_on = true;
  unsigned long ms = millis();
  if (trigger_go(ms))
  {
    if (settings_is_ext_clk())
    {
      is_triggered = false;
    }
    trigger_control.delay = selected_fxn->get_param(led_on ? STRETCH_ON_TIME : STRETCH_OFF_TIME);
    unsigned int my_delay = trigger_control.calc_delay(0);
    trigger_control.next_time = ms + my_delay;
    // trigger_control.next_time = ms;

    gate.put(led_on);
    led_on = !led_on;
    if (led_on)
    {
      trigger_control.pulse_count--;
    }

    uint16_t aval = selected_fxn->get_param(led_on ? STRETCH_LVL1 : STRETCH_LVL2);
    cv_out_scaled(&aval);
    // ui.terminal_debug("My delay: " + String(my_delay) + " Pulse count: " + String(trigger_control.pulse_count));

    if (trigger_control.pulse_count == 0)
    {
      // spank_engine.delay = longest_pulse;
      trigger_control.pulse_count = selected_fxn->get_param(STRETCH_NUM_PULSES);
      trigger_control.next_time = millis() + selected_fxn->get_param(STRETCH_INIT_DELAY);
      reset_trigger();
    }
  }
}

void set_stretch_trigger()
{
  trigger_control.delay = selected_fxn->get_param(STRETCH_ON_TIME);
  trigger_control.pulse_count = selected_fxn->get_param(STRETCH_NUM_PULSES);
  trigger_control.next_time = millis() + selected_fxn->get_param(STRETCH_INIT_DELAY);
  trigger_control.triggered = true;
  trigger_fxn = stretch_trigger;
  // Serial.println("Set Stretch Trigger: " + String(spank_engine.delay));
}

void stretch_fxn()
{
  selected_fxn = &stretch_spanker;
  stretch_spanker.display();
  trigger_control.triggered = false;
  trigger_fxn = stretch_trigger;
}

void stretch_begin()
{
  stretch_spanker.begin();
  stretch_spanker.trigger_fxn = set_stretch_trigger;
}
