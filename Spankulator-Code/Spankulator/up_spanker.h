uint16_t _up_params[5];
uint16_t _up_mins[] = {2, 2, 2, 0, 0};
uint16_t _up_maxs[] = {9999, 9998, 99, 99, 9999};
uint16_t _up_init_vals[] = {100, 10, 10, 0, 100};
uint16_t *up_stuff[] = {_up_params, _up_mins, _up_maxs, _up_init_vals};
String up_labels[] = {"End Pulse Len: ", "Start Pulse Len: ", "Num Pulses: ", "Randomness: ", "Initial Delay: "};
Greenface_gadget up_spanker("Up", up_labels, up_stuff, sizeof(_up_params) / sizeof(_up_params[0]));

void up_trigger()
{
  // Serial.print("Doing up trigger at: "+String(millis())+" -- ");
  // Serial.println(selected_fxn->name);
  // if(f==NULL) f = up_spanker;
  static float the_delay = 0;
  float longest_pulse = float(selected_fxn->get_param(LONGEST_PULSE));
  float shortest_pulse = selected_fxn->get_param(SHORTEST_PULSE);
  float the_swell = (longest_pulse - shortest_pulse) / selected_fxn->get_param(NUM_PULSES);
  // Serial.println("Longest: " + String(longest_pulse) + " Shortest: " + String(shortest_pulse));
  int rnd = selected_fxn->get_param(RANDOMNESS);
  if (!doing_trigger)
  {
    doing_trigger = true;
    the_delay = shortest_pulse;
    do_delay(selected_fxn->get_param(INITIAL_DELAY));
  }
  send_one_pulse(the_delay, longest_pulse, shortest_pulse, rnd, the_swell);
  the_delay += the_swell * 1.1;
  if (the_delay >= longest_pulse)
  {
    doing_trigger = false;
    the_delay = shortest_pulse;
    do_toggle();
  }
}

void up_fxn()
{
  selected_fxn = &up_spanker;
  selected_fxn->display();
}

void up_begin()
{
  // Serial.println("UP Begin!");
  up_spanker.begin();
  up_spanker.trigger_fxn = up_trigger;
  up_spanker.check_params = true;
}
