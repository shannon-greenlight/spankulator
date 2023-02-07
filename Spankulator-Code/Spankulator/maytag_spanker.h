uint16_t _maytag_params[3];
uint16_t _maytag_mins[] = {1, 0, 1};
uint16_t _maytag_maxs[] = {9999, 9999, 99};
uint16_t _maytag_init_vals[] = {100, 500, 20};
uint16_t *maytag_stuff[] = {_maytag_params, _maytag_mins, _maytag_maxs, _maytag_init_vals};
String maytag_labels[] = {"Pulse Length: ", "Delay: ", "#Pulses: "};
Greenface_gadget maytag_spanker("Maytag", maytag_labels, maytag_stuff, sizeof(_maytag_params) / sizeof(_maytag_params[0]));

enum
{
  MAYTAG_PULSE_LEN = 0,
  MAYTAG_DELAY,
  MAYTAG_NUM_PULSES,
};

void maytag_trigger()
{
  static bool led_on = true;
  unsigned long ms = millis();
  if (trigger_go(ms))
  {
    if (settings_is_ext_clk())
    {
      is_triggered = false;
    }
    trigger_control.delay = selected_fxn->get_param(MAYTAG_PULSE_LEN);
    unsigned int my_delay = trigger_control.calc_delay(100);
    trigger_control.next_time = ms + my_delay;

    gate.put(led_on);
    led_on = !led_on;
    if (led_on)
    {
      trigger_control.pulse_count--;
    }

    uint16_t aval = max(0, (my_delay / trigger_control.delay * DAC_FS) - DAC_FS / 2);
    cv_out_scaled(&aval);

    if (trigger_control.pulse_count == 0)
    {
      // spank_engine.delay = longest_pulse;
      trigger_control.delay = selected_fxn->get_param(MAYTAG_PULSE_LEN);
      trigger_control.pulse_count = selected_fxn->get_param(MAYTAG_NUM_PULSES);
      trigger_control.next_time = millis() + selected_fxn->get_param(MAYTAG_DELAY);
      reset_trigger();
    }
  }
}

void set_maytag_trigger()
{
  trigger_control.delay = selected_fxn->get_param(MAYTAG_DELAY);
  trigger_control.pulse_count = selected_fxn->get_param(MAYTAG_NUM_PULSES);
  trigger_control.next_time = millis() + selected_fxn->get_param(MAYTAG_DELAY);
  trigger_control.triggered = true;
  trigger_fxn = maytag_trigger;
  // Serial.println("Set Maytag Trigger: " + String(trigger_control.delay));
}

void maytag_fxn()
{
  selected_fxn = &maytag_spanker;
  maytag_spanker.display();
  trigger_control.triggered = false;
  trigger_fxn = maytag_trigger;
}

void maytag_begin()
{
  maytag_spanker.begin();
  maytag_spanker.trigger_fxn = set_maytag_trigger;
}
