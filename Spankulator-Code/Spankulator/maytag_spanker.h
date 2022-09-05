uint16_t _maytag_params[3];
uint16_t _maytag_mins[] = {1, 0, 1};
uint16_t _maytag_maxs[] = {9999, 9999, 99};
uint16_t _maytag_init_vals[] = {100, 500, 20};
uint16_t *maytag_stuff[] = {_maytag_params, _maytag_mins, _maytag_maxs, _maytag_init_vals};
String maytag_labels[] = {"Pulse Length: ", "Delay: ", "#Pulses: ", "Level 1: ", "Level 2: "};
SPANK_fxn maytag_spanker("Maytag", maytag_labels, maytag_stuff, sizeof(_maytag_params) / sizeof(_maytag_params[0]), &ui);

void maytag_trigger()
{
  static int pulse_ctr = 1;

  // perhaps we're coming from an aborted trigger
  if (!doing_trigger)
  {
    pulse_ctr = 1;
  }

  // Serial.println("Doing Maytag trigger");
  doing_trigger = true;
  float max_len = selected_fxn->get_param(0);
  int delay = selected_fxn->get_param(1);
  int num_pulses = selected_fxn->get_param(2);
  if (pulse_ctr == 1)
  {
    do_delay(delay);
  }
  int my_len = random(10, max_len);
  int aval1 = max(0, my_len * DAC_FS / max_len);
  aval1 = min(DAC_FS, aval1);
  scale_and_offset(&aval1);
  do_pulse(my_len, max_len - my_len, aval1, aval1);

  // Serial.print("Num Pulses left:");
  // Serial.println(num_pulses - i);
  if (pulse_ctr++ >= num_pulses)
  {
    do_toggle();
    doing_trigger = false;
    pulse_ctr = 1;
  }
}

void maytag_fxn()
{
  selected_fxn = &maytag_spanker;
  maytag_spanker.display();
}

void maytag_begin()
{
  maytag_spanker.begin();
  maytag_spanker.trigger_fxn = maytag_trigger;
}
