uint16_t _dn_params[5];
uint16_t _dn_mins[] = {10, 2, 2, 0, 0};
uint16_t _dn_maxs[] = {9999, 9998, 99, 99, 9999};
uint16_t _dn_init_vals[] = {100, 10, 10, 0, 100};
uint16_t *dn_stuff[] = {_dn_params, _dn_mins, _dn_maxs, _dn_init_vals};
String dn_labels[] = {"Start Pulse Len: ", "End Pulse Len: ", "Num Pulses: ", "Randomness: ", "Initial Delay: "};
Greenface_gadget dn_spanker("Down", dn_labels, dn_stuff, sizeof(_dn_params) / sizeof(_dn_params[0]));

void dn_trigger()
{
  // Serial.println("Doing dn trigger");
  static float the_delay = 0;
  float longest_pulse = float(selected_fxn->get_param(LONGEST_PULSE));
  float shortest_pulse = selected_fxn->get_param(SHORTEST_PULSE);
  float the_swell = (shortest_pulse - longest_pulse) / selected_fxn->get_param(NUM_PULSES);
  // Serial.println("Longest: " + String(longest_pulse) + " Shortest: " + String(shortest_pulse));
  int rnd = selected_fxn->get_param(RANDOMNESS);
  if (!doing_trigger)
  {
    doing_trigger = true;
    the_delay = longest_pulse;
    do_delay(selected_fxn->get_param(INITIAL_DELAY));
  }
  send_one_pulse(the_delay, longest_pulse, shortest_pulse, rnd, the_swell);
  the_delay += the_swell * 1.1;
  if (the_delay <= shortest_pulse)
  {
    doing_trigger = false;
    the_delay = longest_pulse;
    do_toggle();
  }
}

void dn_fxn()
{
  selected_fxn = &dn_spanker;
  dn_spanker.display();
}

void dn_begin()
{
  dn_spanker.begin();
  dn_spanker.trigger_fxn = dn_trigger;
  dn_spanker.check_params = true;
}
