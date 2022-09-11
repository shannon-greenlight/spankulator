uint16_t _stretch_params[6];
uint16_t _stretch_mins[] = {1, 1, 1, 0, 0, 0};
uint16_t _stretch_maxs[] = {9999, 9999, 99, DAC_FS, DAC_FS, 9999};
uint16_t _stretch_init_vals[] = {10, 100, 1, DAC_FS, 512, 1000};
uint16_t *stretch_stuff[] = {_stretch_params, _stretch_mins, _stretch_maxs, _stretch_init_vals};
String stretch_labels[] = {"On Time: ", "Off time: ", "#Pulses: ", "Level 1: ", "Level 2: ", "Init Delay: "};
Greenface_gadget stretch_spanker("Stretch", stretch_labels, stretch_stuff, sizeof(_stretch_params) / sizeof(_stretch_params[0]));

void stretch_trigger()
{
  static int pulse_ctr = 1;

  // perhaps we're coming from an aborted trigger
  if (!doing_trigger)
  {
    pulse_ctr = 1;
  }

  doing_trigger = true;
  int on_time = selected_fxn->get_param(0);
  int off_time = selected_fxn->get_param(1);
  int num_pulses = selected_fxn->get_param(2);
  int aval1 = selected_fxn->get_param(3);
  int aval2 = selected_fxn->get_param(4);
  int init_delay = selected_fxn->get_param(5);

  if (pulse_ctr == 1)
  {
    do_delay(init_delay);
  }
  do_pulse(on_time, off_time, aval1, aval2);
  if (pulse_ctr++ >= num_pulses)
  {
    do_toggle();
    doing_trigger = false;
    pulse_ctr = 1;
  }
}

void stretch_fxn()
{
  selected_fxn = &stretch_spanker;
  stretch_spanker.display();
}

void stretch_begin()
{
  stretch_spanker.begin();
  stretch_spanker.trigger_fxn = stretch_trigger;
}
