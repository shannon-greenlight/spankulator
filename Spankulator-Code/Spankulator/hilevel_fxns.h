FunctionPointer functionPointers[] = {up_fxn, dn_fxn, stretch_fxn, toggle_fxn, maytag_fxn, lfo_fxn, user_fxn, dvm_fxn, enter_wifi, settings_fxn};
int num_fxns = sizeof(functionPointers) / sizeof(functionPointers[0]);
void exe_fxn()
{
  (*functionPointers[fxn.get()])();
}

boolean is_lfo()
{
  return fxn.get() == LFO_FXN;
}

boolean is_dvm()
{
  return fxn.get() == DVM_FXN;
}

boolean key_held_down = false;

void restore_display()
{
  keypress = 0;
  ui.display_on();
  new_fxn();
  ui.reset_inactivity_timer();
}

void check_display_restore()
{
  if (fp_key_pressed && !ui.display_is_on && keypress != '!')
  {
    restore_display();
  }
}

// terminal routiines
void terminal_print_status();

void set_repeat_display(bool show_terminal = false)
{
  digitalWrite(repeat_led_pin, repeat_on.get());
  if (show_terminal)
  {
    terminal_print_status();
  }
}

// initialization
void init_all()
{
  // ui.terminal_debug("Factory Reset");
  ui.clearDisplay();
  ui.printLine("Resetting", 0);
  ui.printLine("To Factory Defaults", LINE_1, 1);
  adc_init();
  wifi_init();
  up_spanker.init();
  dn_spanker.init();
  stretch_spanker.init();
  toggle_spanker.init();
  maytag_spanker.init();
  lfo_spanker.init();
  settings_spanker.init();
  settings_init();
  user_init();
  fxn.put(0);
  fxn.write_int(EEPROM_INIT_FLAG, EEPROM_INIT_PATTERN);
  fxn.write_int(EEPROM_OFFSET_FLAG, Greenface_EEPROM::eeprom_offset);
  repeat_on.reset();
  set_repeat_display();
  delay(1500);
  exe_fxn();
}

String fxn_name()
{
  switch (fxn.get())
  {
  case UP_FXN:
    return up_spanker.name;
    break;
  case DN_FXN:
    return dn_spanker.name;
    break;
  case STRETCH_FXN:
    return stretch_spanker.name;
    break;
  case TOGGLE_FXN:
    return toggle_spanker.name;
    break;
  case MAYTAG_FXN:
    return maytag_spanker.name;
    break;
  case LFO_FXN:
    return lfo_spanker.name;
    break;
  case USER_FXN:
    return "User";
    break;
  case DVM_FXN:
    return "Bounce";
    break;
  case WIFI_FXN:
    return "WiFi";
    break;
  case SETTINGS_FXN:
    return "Settings";
    break;
  default:
    // return selected_fxn->name;
    return "Unknown - Fxn: " + String(fxn.get());
  }
}

String list_fxns()
{
  String out = "\"fxns\" : [";
  int fxn_memory = fxn.get();
  for (int i = 0; i < num_fxns; i++)
  {
    fxn.put(i);
    out += enquote(fxn_name());
    // out += toJSON("Fxn_"+String(i),fxn_name());
    if (i < num_fxns - 1)
      out += ",";
  }
  out += "],";
  fxn.put(fxn_memory);
  // Serial.println(out);
  return out;
}

// trigger
void reset_trigger()
{
  do_toggle();
  if (user_doing_trigger)
  {
    trigger_control.triggered = is_triggered = false;
  }
  else
  {
    trigger_control.triggered = is_triggered = repeat_on.get();
  }
}

void do_trigger()
{
  if (!settings_is_ext_clk())
  {
    digitalWrite(triggered_led_pin, HIGH);
  }
  switch (fxn.get())
  {
  case DVM_FXN:
    set_dvm_trigger();
    break;
  case USER_FXN:
    user_do_trigger();
    break;
  case WIFI_FXN:
    repeat_on.reset();
    if (settings_get_wifi() == 2)
    {
      select_wifi_screen = WIFI_IDLE;
      settings_spanker.put_param(1, SETTINGS_WIFI);
      // system_message = "WiFi Reset";
    }
    wifi_activate();
    break;
  case SETTINGS_FXN:
    settings_trigger();
    break;
  default:
    selected_fxn->trigger_fxn();
  }
}

void check_trigger()
{
  if (is_triggered || settings_is_ext_clk())
  {
    if (!trigger_control.triggered)
    {
      // Serial.println("Oh yes you did!");
      do_trigger();
      delay(1);
      // Serial.println("do_trigger: " + String(trigger_control.triggered) + "          " + selected_fxn->name);
    }
    if (settings_is_ext_clk() || fxn.get() == SETTINGS_FXN)
    {
      trigger_control.triggered = true;
      is_triggered = false;
    }
    else
    {
      // is_triggered = repeat_on.get() || trigger_control.triggered || user_doing_trigger;
      is_triggered = repeat_on.get() || trigger_control.triggered || (user_doing_trigger && !settings_is_ext_clk());
      // Serial.println("Delay: " + String(spank_engine.delay));
    }
  }
  else
  {
    digitalWrite(triggered_led_pin, LOW);
    if (!settings_is_ext_clk())
    {
      trigger_control.triggered = false;
    }
  }
}

bool trigger_go(unsigned long ms)
{
  return settings_is_ext_clk() ? is_triggered : trigger_control.triggered && ms >= trigger_control.next_time;
}

void test_trig()
{
  // Serial.println("Testing trigger");
}

void adjust_param(int encoder_val)
{
  static unsigned long last_change_time = 0;
  unsigned long now = millis();
  unsigned long delta = now - last_change_time;
  // Serial.println("Encoder: "+String(encoder_val));
  switch (fxn.get())
  {
  case TOGGLE_FXN:
    selected_fxn->adjust_param(encoder_val, delta);
    toggle_set_output();
    break;
  case LFO_FXN:
    lfo_adjust_param(encoder_val, delta);
    break;
  case DVM_FXN:
    dvm_adjust_param(encoder_val, delta);
    break;
  case USER_FXN:
    user_adjust_param(encoder_val);
    break;
  case SETTINGS_FXN:
    settings_adjust_param(encoder_val, delta);
    break;
  default:
    selected_fxn->adjust_param(encoder_val, delta);
  }
  terminal_print_status();
  // ui.terminal_debug("Encoder delta: " + String(delta));
  last_change_time = now;
}

void check_rotary_encoder()
{
  if (e.is_adjusting())
  {
    ui.reset_inactivity_timer();
    adjust_param(e.getEncoderValue());
    e.resetEncoder();
  }
}

uint16_t get_param(int p_num)
{
  switch (fxn.get())
  {
  case USER_FXN:
  case DVM_FXN:
    return 0;
    break;
  default:
    return selected_fxn->get_param(p_num);
  }
}

String get_label(int i)
{
  switch (fxn.get())
  {
  case WIFI_FXN:
    return "WiFi";
    break;
  case USER_FXN:
    return "User";
    break;
  // case LFO_FXN:
  //   return "LFO";
  //   break;
  default:
    return selected_fxn->get_label(i);
    // Serial.println("Unknown get_label fxn: " + String(fxn.get()));
    // return "?";
  }
}

void put_param(int val)
{
  switch (fxn.get())
  {
  case TOGGLE_FXN:
    selected_fxn->put_param(val);
    toggle_set_output();
    break;
  case DVM_FXN:
    dvm_put_param(val);
    break;
  case USER_FXN:
    user_put_param(val);
    break;
  case LFO_FXN:
    lfo_put_param(val);
    break;
  case SETTINGS_FXN:
    settings_put_param(val);
    // selected_fxn->put_param(val);
    // set_encoder();
    break;
  default:
    selected_fxn->put_param(val);
  }
}

int get_dig_num()
{
  switch (fxn.get())
  {
  case DVM_FXN:
    return dvm_dig_num;
    break;
  case USER_FXN:
    return user_get_dig_num();
  default:
    return selected_fxn->digit_num;
  }
}

int inc_dig_num()
{
  switch (fxn.get())
  {
  case DVM_FXN:
    break;
  case USER_FXN:
    if (key_held_down)
    {
      user_insert_char(' ');
      do
      {
      } while (!all_buttons_up());
      delay(100);
    }
    else
    {
      user_inc_dig_num_by(1);
    }
    break;
  default:
    if (selected_fxn->get_param_type(selected_fxn->param_num) == SPANK_STRING_VAR_TYPE && key_held_down)
    {
      // insert new char
      selected_fxn->insert_char(' ');
    }
    else
    {
      selected_fxn->inc_dig_num_by(1);
    }
  }
}

void dec_dig_num()
{
  switch (fxn.get())
  {
  case DVM_FXN:
    break;
  case USER_FXN:
    if (key_held_down)
    {
      user_remove_char();
      // user_inc_dig_num_by(-1);
      do
      {
      } while (!all_buttons_up());
      delay(100);
    }
    else
    {
      user_inc_dig_num_by(-1);
    }
    break;
  default:
    if (selected_fxn->get_param_type(selected_fxn->param_num) == SPANK_STRING_VAR_TYPE && key_held_down)
    {
      // remove char
      selected_fxn->remove_char();
    }
    else
    {
      selected_fxn->inc_dig_num_by(-1);
    }
  }
}

void put_dig_num(int val)
{
  switch (fxn.get())
  {
  case DVM_FXN:
    break;
  case USER_FXN:
    user_put_dig_num(val);
    break;
  default:
    selected_fxn->put_dig_num(val);
  }
}

uint16_t get_param_num()
{
  switch (fxn.get())
  {
  case USER_FXN:
    return user_get_param_num();
    // user_end();
    break;
  case LFO_FXN:
    return lfo_spanker.param_num;
    break;
  case DVM_FXN:
    // do nothing
    return 0;
    break;
  default:
    return selected_fxn->param_num;
  }
}

void put_param_num(uint16_t pnum)
{
  switch (fxn.get())
  {
  case USER_FXN:
    user_put_param_num(pnum);
    // user_end();
    break;
  case DVM_FXN:
    // dvm_put_param_num(pnum);
    // exe_fxn();
    break;
  default:
    selected_fxn->put_param_num(pnum);
  }
}

void inc_param_num()
{
  switch (fxn.get())
  {
  case USER_FXN:
    user_inc_param_num();
    // user_end();
    break;
  case DVM_FXN:
    dvm_inc_param_num_by(1);
    // exe_fxn();
    break;
  default:
    selected_fxn->inc_param_num_by(1);
    // terminal_print_status();
    // Serial.println("Default inc_param_num fxn: " + String(fxn.get()) + " " + selected_fxn->param_num);
  }
}

void dec_param_num()
{
  switch (fxn.get())
  {
  case USER_FXN:
    user_dec_param_num();
    // user_home();
    break;
  case DVM_FXN:
    dvm_inc_param_num_by(-1);
    // exe_fxn();
    break;
  default:
    selected_fxn->inc_param_num_by(-1);
    terminal_print_status();
  }
}

String get_control(String control)
{
  switch (fxn.get())
  {
  case STRETCH_FXN:
  case TOGGLE_FXN:
  case WIFI_FXN:
  case SETTINGS_FXN:
    return "disabled";
    break;
  default:
    if (control == "scale")
    {
      return String((int)(scale * ADC_FS + .5));
    }
    else if (control == "offset")
    {
      return String(offset_adj);
    }
  }
}

void housekeep()
{
  switch (fxn.get())
  {
  case LFO_FXN:
    lfo_housekeep();
    break;
  case DVM_FXN:
    dvm_housekeep();
    break;
  }
}

void trigInt()
{
  static bool output = 0;
  output = !output;
  digitalWrite(repeat_led_pin, output);
}

void set_encoder()
{
  switch (settings_get_encoder_type())
  {
  case 0: // bourns
    e.msb_pin = 3;
    e.lsb_pin = 2;
    break;
  case 1: // amazon
    e.msb_pin = 2;
    e.lsb_pin = 3;
    break;
  }

  // #define ENCODER_MSB 2
  // #define ENCODER_LSB 3
  // e.msb_pin=ENCODER_MSB;
  // e.lsb_pin=ENCODER_LSB;

  // now set up interrupts
  // attachInterrupt(digitalPinToInterrupt(left_button_pin), trigInt, RISING);
  attachInterrupt(digitalPinToInterrupt(e.lsb_pin), intFxnB, RISING);
  attachInterrupt(digitalPinToInterrupt(e.msb_pin), intFxnA, RISING);
}

// void terminal_print_status()
// {
//   ui.terminal_print_status(repeat_on.get(), triggered, scale, offset);
// }

void set_repeat_on(bool val)
{
  if (val)
  {
    repeat_on.set();
  }
  else
  {
    repeat_on.reset();
  }
  set_repeat_display(true);
}

void new_fxn()
{
  // ui.clearDisplay();
  pinMode(gate_out_pin, OUTPUT); // dvm changes this to PWM, put it back
  is_triggered = user_doing_trigger = false;
  wifi_ui_message = "";
  reset_triggers();
  settings_set_ext_trig();
  exe_fxn();
  terminal_print_status();
  wifi_new_fxn();
}

#define TRIGGER_MASK 0x80
void hilevel_debug()
{
  for (int i = 0; i < LFO_PARTS; i++)
  {
    if (i % 8 == 0)
    {
      Serial.println("");
    }
    Serial.print(wave_table[i], HEX);
    Serial.print(" ");
  }
  return;
  adc_summary();
  DAC->CTRLB.reg &= 0x3f; // use EXT as the reference
  DAC->CTRLB.reg |= 0x80; // use EXT as the reference
  Serial.print("Reg B: ");
  Serial.println(DAC->CTRLB.reg, HEX);
  return;
  // ui.terminal_debug(" Gain: " + String(gainCorrectionValue.get()) + " Offset: " + String(offsetCorrectionValue.get()));
  //  return;
  fxn.test();
  return;

  int port_a = 0;
  int port_b = 0;
  port_a = PORT->Group[PORTA].IN.reg; // PORT->Group[PORTA].IN.reg
  port_b = REG_PORT_IN1 & 0xFFFF;
  char buf[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // buffer to print up to 9 digits
  // int inputs = REG_PORT_IN0;
  //  printf("%04X", inputs); // gives 12AB
  snprintf(buf, 6, "%04X", port_a);
  ui.terminal_debug("Port A: " + String(buf));
  snprintf(buf, 6, "%04X", port_b);
  Serial.println("\nPort B: " + String(buf));
  // ui.terminal_debug("keypress: " + String(keypress));
  // ui.printLine("0123456789012345678901234", ui.lines[1], 1);
}

void send_user_params_to_USB()
{
  byte temp = user_dig_num;
  String out = "";
  ui.t.print("\"params\" : [");
  for (user_dig_num = 0; user_dig_num < user_string.length() && user_dig_num < USER_MAXLEN; user_dig_num++)
  {
    // Serial.print(user_dig_num);
    // Serial.print(". ");
    // Serial.print(user_string.charAt(user_dig_num));
    // Serial.println("*");
    // Serial.println(out);
    if (user_string.charAt(user_dig_num) != ' ')
    {
      if (user_dig_num > 0)
      {
        out += ",";
      }
      out += user_params_toJSON();
    }
    ui.t.print(out);
    // Serial.print(out);
    out = "";
  }
  ui.t.print("]");
  user_dig_num = temp;
}

bool sending_to_USB;
void send_status_to_USB()
{
  if (!sending_to_USB)
  {
    sending_to_USB = true;
    ui.t.print("{");
    ui.t.print(toJSON("status", "trigger"));
    ui.t.print(",");
    ui.t.print(toJSON("triggered", is_triggered ? "ON" : "OFF"));
    ui.t.print(",");
    ui.t.print(toJSON("gate", String(gate.get() ? "ON" : "OFF")));
    ui.t.print(",");
    ui.t.print(toJSON("toggle", String(tog.get() ? "ON" : "OFF")));
    ui.t.print("}");
    // This terminates serialport message
    ui.t.println("\r\n\r\n");
    sending_to_USB = false;
  }
}

// savced just in case
// const char *underline_chars[2] = {"-", "^"};
// int underline_char_index = 0;

void send_data_to_USB(char cmd)
{
  if (sending_to_USB || !is_usb_direct())
    return;
  sending_to_USB = true;
  ui.t.print("{");
  if (cmd == '[')
    return;
  ui.t.print(toJSON("fxn", fxn_name()));
  ui.t.print(",");
  ui.t.print(toJSON("fxn_num", String(fxn.get())));
  ui.t.print(",");
  if (true || cmd == ' ' || cmd == 'f' || cmd == '+' || cmd == '-' || cmd == 'U')
    ui.t.print(list_fxns());
  ui.t.print(toJSON("device_name", settings_get_device_name()));
  ui.t.print(",");
  ui.t.print(toJSON("quantized", settings_is_quantized() ? "Quantized" : "Normal"));
  ui.t.print(",");
  ui.t.print(toJSON("repeat_on", repeat_on.get() ? "ON" : "OFF"));
  ui.t.print(",");
  ui.t.print(toJSON("triggered", is_triggered ? "ON" : "OFF"));
  ui.t.print(",");
  ui.t.print(toJSON("cmd", String(cmd)));
  ui.t.print(",");
  // ui.t.print(toJSON("underline_char", String(underline_chars[underline_char_index])));
  // ui.t.print(",");
  ui.t.print(toJSON("digit_num", String(get_dig_num())));
  ui.t.print(",");
  ui.t.print(toJSON("param_num", String(get_param_num())));
  ui.t.print(",");
  ui.t.print(toJSON("param_active", String(selected_fxn->get_param_active())));
  ui.t.print(",");
  ui.t.print(toJSON("dp", (selected_fxn->decimal_places) ? String(selected_fxn->decimal_places[selected_fxn->param_num]) : "0"));
  ui.t.print(",");
  ui.t.print(toJSON("adj", String(adj)));
  ui.t.print(",");
  ui.t.print(toJSON("cv_val", String(cv_val)));
  ui.t.print(",");
  // ui.t.print(toJSON("scale", String((int)(scale * ADC_FS + .5))));
  ui.t.print(toJSON("scale", get_control("scale")));
  ui.t.print(",");
  // ui.t.print(toJSON("offset", String(offset_adj)));
  ui.t.print(toJSON("offset", get_control("offset")));
  ui.t.print(",");
  ui.t.print(toJSON("gate", String(gate.get() ? "ON" : "OFF")));
  ui.t.print(",");
  ui.t.print(toJSON("toggle", String(tog.get() ? "ON" : "OFF")));
  ui.t.print(",");
  ui.t.print(toJSON("dac_fs", String(DAC_FS)));
  ui.t.print(",");
  ui.t.print(toJSON("adc_fs", String(ADC_FS)));
  ui.t.print(",");
  ui.t.print(toJSON("clock", !settings_is_ext_clk() ? "internal" : "external"));
  ui.t.print(",");
  ui.t.print(toJSON("trigger_enable", get_trigger_enable() ? "enabled" : "disabled"));
  ui.t.print(",");
  // ui.t.print(toJSON("ext_trigger_disable", get_ext_trigger_disable() ? "disabled" : "enabled"));
  ui.t.print(toJSON("ext_trigger_disable", settings_get_ext_trig() == 0 ? "enabled" : "disabled"));
  ui.t.print(",");

  if (fxn_name() == "User")
  {
    // Serial.println("Sending: "+fxn_name());
    ui.t.print("\"sequence\" : {");
    ui.t.print(toJSON("label", "Sequence"));
    ui.t.print(",");
    ui.t.print(toJSON("type", "sequence"));
    ui.t.print(",");
    ui.t.print(toJSON("value", user_string.get()));
    ui.t.print(",");
    ui.t.print(toJSON("legal_values", user_ops));
    ui.t.print(",");
    ui.t.print(toJSON("dig_num", String(user_dig_num)));
    ui.t.print(",");
    ui.t.print(toJSON("selected", (user_entering_param == 0 && selected_fxn->param_num == 0) ? "true" : "false"));
    ui.t.print(",");
    send_user_params_to_USB();
    ui.t.print("},");
  }
  else
  {
    user_entering_param = 1; // if not user mode make sure we aren't missing selected param
  }

  if (wifi_ui_message > "" || true)
  {
    ui.t.print(toJSON("message", wifi_ui_message));
    ui.t.print(",");
  }

  if (fxn_name() == "Bounce")
  {
    ui.t.print(toJSON("meas", dvm_meas));
    ui.t.print(",");
  }
  else
  {
    // clear out old message
    wifi_ui_message = "";
  }

  ui.t.print("\"params\" : [");
  // fix this for other fxns
  ui.t.print(params_toJSON());
  ui.t.print("]");
  ui.t.print("}");

  // The HTTP response ends with another blank line:
  // ui.t.println("");

  // This terminates serialport message
  ui.t.println("\r\n\r\n");
  sending_to_USB = false;
}

void send_data_to_apps(char c)
{
  if (is_usb_direct())
  {
    send_data_to_USB(c);
  }
  if (is_wifi_connected())
  {
    // send_data_to_client(client, c);
  }
}

void check_serial()
{
  static bool entering_string = false;

  if (Serial.available() > 0)
  {
    char c = Serial.read();
    // ui.terminal_debug(in_str + " .. " + String(c));
    if (c == '$' && selected_fxn->param_is_stringvar())
    {
      entering_string = true;
      selected_fxn->set_cursor_to_param();
    }
    if (!entering_string && (c == '&' || c == 'i' || c == 127 || c == '~' || c == '*' || c == '!' || c == 'u' || c == 'd' || c == '+' || c == '-' || c == 'z' || c == 'Z' || esc_mode))
    {
      // process this immediately with process_keypress
      keypress = c;
    }
    else
    {
      if (c == '\r')
      {
        // process this with process_cmd
        cmd_available = true;
        entering_string = false;
      }
      else
      {
        if (entering_string && c != '$')
        {
          ui.terminal_print(c); // echo string to terminal
        }
        if (c == 27 || c == '[')
        {
          esc_mode = true;
        }
        else
        {
          in_str += c;
        }
      }
    }
  }
}

// boolean monitor = false;
void check_keyboard()
{
  // gate.toggle(); // gate.set();
  // int inactivity_timeout = settings_get_inactivity_timeout();
  if (!keypress)
  {
    if (!digitalRead(up_button_pin))
      keypress = '+'; // 62
    else if (!digitalRead(dn_button_pin))
      keypress = '-'; // 60
    else if (!digitalRead(repeat_button_pin))
      keypress = '&'; // 42
    else if (!digitalRead(trigger_button_pin))
    // else if ((port_a & TRIGGER_MASK) == 0)
    {
      keypress = '!'; // 33
    }
    else if (!digitalRead(param_up_button_pin))
    {
      esc_mode = true;
      keypress = 'A'; // up 65
    }
    else if (!digitalRead(param_dn_button_pin))
    {
      esc_mode = true;
      keypress = 'B'; // dn 66
    }
    else if (!digitalRead(right_button_pin))
    {
      esc_mode = true;
      keypress = 'C'; // 67
    }
    else if (!digitalRead(left_button_pin))
    {
      esc_mode = true;
      keypress = 'D'; // 68
    }

    fp_key_pressed = keypress != 0 && keypress != '!';

    if (keypress && keypress != '!')
    {
      ui.reset_inactivity_timer();
    }
    else
    {
      // inc_inactivity_timer();
    }
  }
  else
  {
    if (all_buttons_up())
    {
      keypress = 0;
    }
  }
  // gate.reset();
}

void process_keypress()
{
  // ui.terminal_debug("Processing keypress: " + String(keypress));
  unsigned long started = millis();
  if (keypress != '!')
  {
    do
    {
      key_held_down = millis() - started > 2000;
    } while (!all_buttons_up() && !key_held_down);
  }

  switch (keypress)
  {
  case '*':
    fxn.put(SETTINGS_FXN);
    new_fxn();
    break;
  case 'u':
    adjust_param(1);
    break;
  case 'd':
    adjust_param(-1);
    break;
  case '>': // >
  case '+': // +
    fxn.inc();
    new_fxn();
    break;
  case '<': // <
  case '-': // -
    fxn.inc(-1);
    new_fxn();
    break;
  case '&': // *
    if (!settings_is_ext_clk())
    {
      repeat_on.toggle();
      set_repeat_display(true);
      do
      {
      } while (!all_buttons_up());
      delay(25);
    }
    break;
  case 'i':
    if (fxn.get() == USER_FXN)
    {
      user_insert_char(' ');
    }
    else
    {
      selected_fxn->insert_char(' ');
    }
    break;
  case 127:
    if (fxn.get() == USER_FXN)
    {
      user_remove_char();
    }
    else
    {
      selected_fxn->remove_char();
    }
    break;
  case '!': // 33
    // is_triggered = !is_triggered;
    is_triggered = settings_is_ext_clk() ? true : !is_triggered;
    // terminal_print_status();
    // ui.terminal_debug("Triggered: " + String(triggered) + " Button: " + String(digitalRead(trigger_button_pin)));
    // delay(50);
    break;
  case 'A': // up arrow in esc mode A
    if (esc_mode)
    {
      dec_param_num();
    }
    break;
  case 'B': // dn arrow B
    if (esc_mode)
    {
      inc_param_num();
    }
    break;
  case 'C': // right arrow in esc mode C
    if (esc_mode)
    {
      inc_dig_num();
    }
    break;
  case 'D': // left arrow D
    if (esc_mode)
    {
      dec_dig_num();
    }
    break;
  case '~':
    hilevel_debug();
    break;
  case 'Z': // disable display and terminal
    // ui.terminal_debug("Turning off display...");
    ui.all_off();
    exe_fxn();
    break;
  case 'z': // refresh screen, turn on display and terminal, reset inactivity timer
    ui.reset_inactivity_timer();
    ui.all_on();
    exe_fxn();
    terminal_print_status();
    break;
  }
  esc_mode = keypress == 27 || keypress == '[';
  housekeep();

  if (keypress == '!')
    reset_triggers();
  keypress = 0;
  key_held_down = false;
  // Serial.println("Esc Mode: "+String(esc_mode));
}

void process_cmd(String in_str)
{
  // Serial.println("Process cmd: " + in_str);
  // ui.terminal_debug("Process cmd: " + in_str);
  // noInterrupts();
  char cmd = in_str.charAt(0);
  int int_param = in_str.substring(1).toInt();
  uint16_t aval;
  remote_adjusting = false;

  switch (cmd)
  {
  case '{':
    gainCorrectionValue.put(int_param);
    analogReadCorrection(offsetCorrectionValue.get(), gainCorrectionValue.get());
    adc_summary();
    break;
  case ';':
    MyTimer5.begin(int_param); // beats per sec
    break;
  case '[':
    esc_mode = true;
    cmd = in_str.charAt(1);
    break;
  case 's':
    if (fxn.get() == USER_FXN)
    {
      user_select_sequence();
    }
    break;
  case ':':
    put_dig_num(int_param);
    exe_fxn();
    break;
  case 'p':
    put_param_num(int_param);
    exe_fxn();
    break;
  case 'q':
    settings_put_quantized(int_param);
    terminal_print_status();
    break;
  case 'O':
    if (settings_get_pot_fxn() != SETTING_CV_OFFSET && settings_get_dc_fxn() != SETTING_CV_OFFSET)
    {
      offset_adj = int_param;
      set_scale();
      remote_adjusting = true;
      housekeep();
      terminal_print_status();
    }
    break;
  case 'P':
    settings_spanker.param_put(int_param, SETTINGS_POT_FXN);
    do_cv_mod(); // ?? dvm does this in trigger fxn
    terminal_print_status();
    break;
  case 'V':
    settings_spanker.param_put(int_param, SETTINGS_DC_FXN);
    terminal_print_status();
    break;
  case 'S':
    if (settings_get_pot_fxn() != SETTING_CV_SCALE && settings_get_dc_fxn() != SETTING_CV_SCALE)
    {
      scale_adj = int_param;
      set_scale();
      remote_adjusting = true;
      housekeep();
      terminal_print_status();
    }
    break;
  case 'c':
    if (settings_get_pot_fxn() != SETTING_CV_VALUE && settings_get_dc_fxn() != SETTING_CV_VALUE)
    {
      cv_out(int_param); // unscaled
    }
    break;
  case 'C':
    if (settings_get_pot_fxn() != SETTING_CV_VALUE && settings_get_dc_fxn() != SETTING_CV_VALUE)
    {
      aval = int_param;
      cv_out_scaled(&aval);
    }
    break;
  case 'f':
    fxn.put(int_param);
    new_fxn();
    break;
  case 'r':
    // ui.terminal_debug("Process cmd: " + in_str + " Param: " + String(int_param));
    switch (int_param)
    {
    case 0:
      repeat_on.reset();
      break;
    case 1:
      repeat_on.set();
      break;
    case 2:
      repeat_on.toggle();
      break;
    default:
      repeat_on.reset();
    }
    set_repeat_display(true);
    break;
  case 't':
    switch (int_param)
    {
    case 0:
      is_triggered = false;
      break;
    case 1:
      is_triggered = true;
      break;
    case 2:
      is_triggered = !is_triggered;
      break;
    default:
      is_triggered = false;
    }
    terminal_print_status();
    break;
  case 'T':
    switch (int_param)
    {
    case 0:
      tog.reset();
      break;
    case 1:
      tog.set();
      break;
    case 2:
      tog.toggle();
      break;
    default:
      Serial.println(F("Illegal value in process_cmd: Set Toggle"));
    }
    terminal_print_status();
    break;
  case 'G':
    switch (int_param)
    {
    case 0:
      gate.reset();
      break;
    case 1:
      gate.set();
      break;
    case 2:
      gate.toggle();
      break;
    default:
      Serial.println(F("Illegal value in process_cmd: Set Gate"));
    }
    terminal_print_status();
    break;
  case 'U':
    settings_put_usb_direct(int_param);
    break;
  case 'J':
    switch (int_param)
    {
    case 0:
      ui.display_off();
      break;
    case 1:
      ui.display_on();
      break;
    }
    break;
  case 'K':
    // Serial.println("Set CLK");
    switch (int_param)
    {
    case 0:
      is_triggered = false;
      settings_put_ext_clk(0);
      break;
    case 1:
      settings_put_ext_clk(1);
      break;
    case 2:
      settings_toggle_ext_clk();
      break;
    }
    terminal_print_status();
    break;
  case 'D':
    switch (int_param)
    {
    case 0:
      disable_triggers();
      break;
    case 1:
      enable_triggers();
      break;
    case 2:
      toggle_trigger_enable();
      break;
    case 3:
      reset_triggers();
      break;
    }
    break;
  case 'E':
    switch (int_param)
    {
    case 0:
    case 1:
      settings_put_ext_trig(int_param);
      break;
    case 2:
      settings_toggle_ext_trig();
      break;
    }
    terminal_print_status();
    break;
  case '$':
    String sval = urlDecode(in_str.substring(1));
    // Serial.println("instr: " + in_str + "*");
    // Serial.println("String val: " + sval + "*");
    if (fxn.get() == USER_FXN && !esc_mode)
    {
      user_update_user_string(in_str);
    }
    else
    {
      selected_fxn->put_string_var(urlDecode(in_str.substring(1)));
    }

    exe_fxn();
    break;
  }

  if (cmd > 47 && cmd < 58)
  {
    put_param(in_str.toInt());
  }

  if (String(cmd) != "\n" && cmd != 'p')
  {
    if (String(cmd) == "%")
    {
      keypress = 27;
    }
    else
    {
      keypress = cmd;
    }

    process_keypress();
  }
  // interrupts();
}

void fxn_begin()
{
  fxn.max = num_fxns - 1;
  // fxn.max=7;

  fxn.begin(true);
  repeat_on.begin(false);

  fxn.xfer();
  repeat_on.xfer();
  // fxn.put(0);
  if (fxn.get() == WIFI_FXN)
    fxn.put(0);

  keypress = 0;
  reset_triggers();

  set_repeat_display();
}

void heartbeat()
{
  (*trigger_fxn)();
}

void heartbeat_begin()
{
  MyTimer5.begin(1000); // beats per sec

  // define the interrupt callback function
  MyTimer5.attachInterrupt(heartbeat);

  // start the timer
  MyTimer5.start();
}

void begin_all()
{
  // Start Serial
  Serial.begin(115200);
  // delay(2000);
  // while (! Serial);

  e.numFxns = 0;
  // e.t = ui.t;
  // e.debug = true;

  adc_config_hardware();

  hardware_begin();
  // heartbeat_begin();
  ui.begin(face1);
  adc_begin();
  up_begin();
  dn_begin();
  stretch_begin();
  toggle_begin();
  maytag_begin();
  lfo_begin();
  user_begin();
  dvm_begin();
  wifi_begin();
  settings_begin();

  fxn_begin();
}
