uint16_t _up_params[5];
uint16_t _up_mins[] = {2, 2, 2, 0, 0};
uint16_t _up_maxs[] = {9999, 9998, 99, 99, 9999};
uint16_t _up_init_vals[] = {100, 10, 10, 0, 100};
uint16_t *up_stuff[] = {_up_params, _up_mins, _up_maxs, _up_init_vals};
String up_labels[] = {"End Pulse Len: ", "Start Pulse Len: ", "Num Pulses: ", "Randomness: ", "Initial Delay: "};
Greenface_gadget up_spanker("Up", up_labels, up_stuff, sizeof(_up_params) / sizeof(_up_params[0]));

void up_trigger()
{
  static bool led_on = true;
  unsigned long ms = millis();
  if (trigger_go(ms))
  {
    if (settings_is_ext_clk())
    {
      is_triggered = false;
    }
    gate.put(led_on);
    led_on = !led_on;

    trigger_control.trigger(TRIG_UP, led_on, ms);

    cv_out_scaled(&trigger_control.aval);
  }
}

void set_up_trigger()
{
  trigger_control.delay = selected_fxn->get_param(SHORTEST_PULSE);
  trigger_control.pulse_count = selected_fxn->get_param(NUM_PULSES);
  trigger_control.next_time = millis() + selected_fxn->get_param(INITIAL_DELAY);
  trigger_control.triggered = true;
  trigger_fxn = up_trigger;
  // Serial.println("Set Up Trigger: " + String(trigger_control.pulse_count));
}

void up_fxn()
{
  selected_fxn = &up_spanker;
  selected_fxn->display();
  trigger_control.triggered = false;
  trigger_fxn = up_trigger;
}

void up_begin()
{
  // Serial.println("UP Begin!");
  up_spanker.begin();
  up_spanker.trigger_fxn = set_up_trigger;
  up_spanker.check_params = true;
}
