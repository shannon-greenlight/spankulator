uint16_t _toggle_params[4];
uint16_t _toggle_mins[] = {1, 0, 0, 0};
uint16_t _toggle_maxs[] = {9999, DAC_FS, DAC_FS, 2};
uint16_t _toggle_init_vals[] = {100, DAC_FS, 0, 0};
uint16_t *toggle_stuff[] = {_toggle_params, _toggle_mins, _toggle_maxs, _toggle_init_vals};
String toggle_string_params[] = {"", "", "", "Toggle,High,Low"};
String toggle_labels[] = {"Delay: ", "Hi Level: ", "Lo Level: ", "State: "};
Greenface_gadget toggle_spanker("Toggle", toggle_labels, toggle_stuff, sizeof(_toggle_params) / sizeof(_toggle_params[0]));

#define TOGGLE_DELAY 0
#define TOGGLE_HI_LEVEL 1
#define TOGGLE_LO_LEVEL 2
#define TOGGLE_STATE 3

void toggle_set_output()
{
  int aval1 = selected_fxn->get_param(gate.get() ? TOGGLE_HI_LEVEL : TOGGLE_LO_LEVEL);
  // Serial.print("Toggle Trigger, aval1: ");
  // Serial.println(aval1);
  //  scale_and_offset(&aval1);
  cv_out(aval1);
}

void toggle_trigger()
{
  triggered = true;
  doing_trigger = true;
  int delay = selected_fxn->get_param(TOGGLE_DELAY);
  do_delay(delay);
  switch (selected_fxn->get_param(TOGGLE_STATE))
  {
  case 0: // toggle
    // ui.terminal_debug("Toggle State: " + String(!gate.get()));
    gate.toggle();
    break;
  case 1: // high
    gate.set();
    break;
  case 2: // low
    gate.reset();
    break;
  }

  toggle_set_output();

  do_toggle();
  doing_trigger = false;
}

void toggle_fxn()
{
  selected_fxn = &toggle_spanker;
  // possibly init toggle state here
  toggle_spanker.display();
}

void toggle_begin()
{
  toggle_spanker.begin();
  toggle_spanker.trigger_fxn = toggle_trigger;
  toggle_spanker.string_params = toggle_string_params;
}
