uint16_t _dn_params[5];
uint16_t _dn_mins[] = {10, 2, 2, 0, 0};
uint16_t _dn_maxs[] = {9999, 9998, 99, 99, 9999};
uint16_t _dn_init_vals[] = {100, 10, 10, 0, 100};
uint16_t *dn_stuff[] = {_dn_params, _dn_mins, _dn_maxs, _dn_init_vals};
String dn_labels[] = {"Start Pulse Len: ", "End Pulse Len: ", "Num Pulses: ", "Randomness: ", "Initial Delay: "};
Greenface_gadget dn_spanker("Down", dn_labels, dn_stuff, sizeof(_dn_params) / sizeof(_dn_params[0]));

void dn_trigger()
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

    trigger_control.trigger(TRIG_DOWN, led_on, ms);

    cv_out_scaled(&trigger_control.aval);
  }
}

void set_dn_trigger()
{
  trigger_control.delay = dn_spanker.get_param(LONGEST_PULSE);
  trigger_control.pulse_count = dn_spanker.get_param(NUM_PULSES);
  trigger_control.next_time = millis() + dn_spanker.get_param(INITIAL_DELAY);
  trigger_control.triggered = true;
  trigger_fxn = dn_trigger;
  // Serial.println("Set Dn Trigger: " + String(trigger_control.delay));
}

void dn_fxn()
{
  selected_fxn = &dn_spanker;
  dn_spanker.display();
  trigger_control.triggered = false;
  trigger_fxn = dn_trigger;
}

void dn_begin()
{
  dn_spanker.begin();
  dn_spanker.trigger_fxn = set_dn_trigger;
  dn_spanker.check_params = true;
}
