// declarations for later
String fxn_name(void);
String list_fxns(void);
void process_cmd(String);
int get_dig_num(void);
byte user_get_dig_num(void);
uint16_t get_param_num(void);
void init_all(void);
// bool wifi_enabled(void);

// useful fxns
// for when you just want to do nothing
void noop() {}

String csv_elem(String s, char delim, int instance)
{
  // vector <String> out;
  int indx = 0;
  int found = 0;
  for (unsigned int i = 0; i < s.length() + 1; i++)
  {
    if (s.charAt(i) == delim || i == s.length())
    {
      if (found == instance)
      {
        return s.substring(indx, i);
      }
      found++;
      indx = i + 1;
    }
  }
  return "CSV element not found!";
}

String urlDecode(String str)
{
  String ret;
  char ch;
  int i, ii, len = str.length();

  for (i = 0; i < len; i++)
  {
    if (str[i] != '%')
    {
      if (false && str[i] == '+') // spank client must not use + for space
        ret += ' ';
      else
        ret += str[i];
    }
    else
    {
      sscanf(str.substring(i + 1, i + 3).c_str(), "%x", &ii);
      // Serial.println("UrlDecode char: " + str + ": " + String(ii) + ".");
      // ii = 32;
      ch = static_cast<char>(ii);
      ret += ch;
      i = i + 2;
    }
  }
  return ret;
}

const char *getfield(char *line, int num)
{
  const char *tok;
  for (tok = strtok(line, ",");
       tok && *tok;
       tok = strtok(NULL, ",\n"))
  {
    if (!--num)
      return tok;
  }
  return NULL;
}

boolean isDifferent(float a, float b)
{
  int ai = int(100 * a);
  int bi = int(100 * b);
  return (ai != bi);
}

String enquote(String value)
{
  return "\"" + value + "\"";
}

String toJSON(String key, String value)
{
  String s1 = "\\";
  String s2 = "\\\\";
  value.replace(s1, s2);

  s1 = "\"";
  s2 = "\\\"";
  value.replace(s1, s2);

  // Serial.println(value);
  return "\"" + key + "\" :\"" + value + "\"";
}

// hardware primitives
boolean user_adjusting()
{
  if (settings_get_pot_fxn() == 0)
  {
    return false;
  }
  else
  {
    int reading = read_dc_filtered(ADC_POT);
    // ui.terminal_debug("Adjusting: " + String(reading));
    if (abs(adj - reading) > 64)
    {
      adj = reading;
      return true;
    }
    else
    {
      return false;
    }
  }
}

boolean read_trigger_pin()
{
  return !digitalRead(trigger_button_pin);
}

boolean all_buttons_up()
{
  // Serial.println(digitalRead(trigger_button_pin));
  return digitalRead(up_button_pin) &&
         digitalRead(dn_button_pin) &&
         digitalRead(repeat_button_pin) &&
         digitalRead(trigger_button_pin) &&
         digitalRead(param_up_button_pin) &&
         digitalRead(param_dn_button_pin) &&
         digitalRead(left_button_pin) &&
         digitalRead(right_button_pin);
}

// a "private" function
bool get_active_state()
{
  return settings_get_board_generation() < 4 ? LOW : HIGH;
}

void reset_triggers()
{
  // on old boards negative pulse, on new boards positive pulse
  // Serial.println("Resetting triggers!");
  bool active_state = get_active_state(); // HIGH for new style boards
  digitalWrite(disable_trigger_pin, active_state);
  delay(1);
  digitalWrite(disable_trigger_pin, !active_state);
  delay(1);
  // digitalWrite(disable_ext_trigger_pin, LOW);
}

void disable_triggers()
{
  bool active_state = get_active_state(); // HIGH for new style boards
  digitalWrite(disable_trigger_pin, active_state);
}

void enable_triggers()
{
  bool active_state = get_active_state(); // HIGH for new style boards
  digitalWrite(disable_trigger_pin, !active_state);
}

bool get_trigger_enable()
{
  bool active_state = get_active_state(); // HIGH for new style boards
  return digitalRead(disable_trigger_pin) != active_state;
}

void toggle_trigger_enable()
{
  if (get_trigger_enable())
  {
    disable_triggers();
  }
  else
  {
    enable_triggers();
  }
}

void disable_ext_trigger()
{
  bool active_state = get_active_state(); // HIGH enables ext trig for new style boards
  digitalWrite(disable_ext_trigger_pin, !active_state);
}

void enable_ext_trigger()
{
  bool active_state = get_active_state(); // HIGH enables ext trig for new style boards
  digitalWrite(disable_ext_trigger_pin, active_state);
}

// true disables ext trigger
void put_disable_ext_trigger(bool val)
{
  bool active_state = !get_active_state(); // HIGH enables ext trig for new style boards
  digitalWrite(disable_ext_trigger_pin, val == active_state);
}

// returns true if ext trigger is disabled
bool get_ext_trigger_disable()
{
  bool active_state = get_active_state(); // HIGH enables ext trig for new style boards
  // Serial.println("Pin: " + String(digitalRead(disable_trigger_pin)) + " State: " + String(active_state));
  return digitalRead(disable_trigger_pin) == active_state;
}

void toggle_disable_ext_trigger()
{
  put_disable_ext_trigger(!get_ext_trigger_disable());
}

void do_toggle()
{
  tog.toggle();
}

// for mcp4811 ??
// void write_dac(uint16_t val) {
//   val = ( val << 2 ) | 0x3000;
//   digitalWrite(dac_select_pin, LOW); //initialize the chip
//   SPI.transfer16(val); // write to device
//   digitalWrite(dac_select_pin, HIGH); // de-initialize the chip
// }

void write_dac(uint16_t val)
{
  // Serial.println("Write DAC: " + String(val));
  analogWrite(aout_pin, val);
}

void set_scale()
{
  scale = (scale_adj / float(ADC_FS));
  offset = offset_adj - scale_adj / 2;
}

void scale_and_offset(uint16_t *val)
{
  int temp = *val;
  temp *= scale;
  temp += offset >> ADC_DAC_SHIFT;
  temp = constrain(temp, 0, DAC_FS);
  *val = temp;
}

// The CV Output circuit inverts the signal, so invert it here
void cv_out(uint16_t val)
{
  // Serial.println("CV Out: " + String(val));
  if (settings_is_quantized())
    val &= 0xFFF8;
  cv_val = val;
  write_dac(DAC_FS - cv_val);
}

void cv_out_scaled(uint16_t *val)
{
  scale_and_offset(val);
  cv_out(*val);
}

void mod_scale(adc_chan chan)
{
  scale_adj = readADC(chan);
}

void mod_offset(adc_chan chan)
{
  offset_adj = readADC(chan);
}

void mod_value(adc_chan chan)
{
  if (!is_triggered)
  {
    cv_out(readADC(chan) >> 2);
  }
}

void do_cv_mod()
{
  // noInterrupts();
  // critical, time-sensitive code here
  int mod_type = 10 * settings_get_dc_fxn() + settings_get_pot_fxn();
  int sig_in;
  switch (mod_type)
  {
  case 0: // all off
    break;
  case 1: // pot scale, dc off
    mod_scale(ADC_POT);
    set_scale();
    break;
  case 2: // pot offset, dc off
    mod_offset(ADC_POT);
    set_scale();
    break;
  case 3: // pot value, dc off
    mod_value(ADC_POT);
    break;
  case 10: // dc scale, pot off
    mod_scale(ADC_DC);
    set_scale();
    break;
  case 11: // dc scale, pot scale
    scale_adj = int(float(readADC(ADC_DC)) / ADC_FS * readADC(ADC_POT));
    set_scale();
    break;
  case 12: // pot offset, dc scale
    mod_offset(ADC_POT);
    mod_scale(ADC_DC);
    set_scale();
    break;
  case 13: // pot value, dc scale
    mod_value(ADC_POT);
    mod_scale(ADC_DC);
    set_scale();
    break;
  case 20: // dc offset, pot off
    mod_offset(ADC_DC);
    set_scale();
    break;
  case 22: // dc offset, pot offset
    offset_adj = readADC(ADC_DC) + readADC(ADC_POT);
    set_scale();
    break;
  case 30: // dc value, pot off
    mod_value(ADC_DC);
    break;
  case 21: // pot scale, dc offset
    mod_scale(ADC_POT);
    mod_offset(ADC_DC);
    set_scale();
    break;
  case 23: // pot value, dc offset
    mod_value(ADC_POT);
    mod_offset(ADC_DC);
    set_scale();
    break;
  case 31: // pot scale, dc value
    mod_scale(ADC_POT);
    mod_value(ADC_DC);
    set_scale();
    break;
  case 32: // pot offset, dc value
    mod_offset(ADC_POT);
    mod_value(ADC_DC);
    set_scale();
    break;
  case 33: // pot value, dc value
    if (!is_triggered)
    {
      sig_in = (readADC(ADC_DC) >> 2) - DAC_FS / 2;
      cv_out(sig_in + readADC(ADC_POT) >> 2);
    }
    break;
  }
  // interrupts();
}