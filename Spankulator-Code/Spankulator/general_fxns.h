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
  static byte cnt = 0;
  int reading = analogRead(ain1_pin);
  if (abs(adj - reading) > 20)
  {
    if (cnt++ > 3)
    {
      cnt = 0;
      adj = reading;
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    cnt = 0;
    return false;
  }
}

void set_scale()
{
  scale = (scale_adj / float(ADC_FS));
  offset = offset_adj - scale_adj / 2;
}

void scale_and_offset(int *val)
{
  *val *= scale;
  *val += offset >> ADC_DAC_SHIFT;
  *val = min(DAC_FS, *val);
  *val = max(0, *val);
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
  switch (BOARD_GENERATION)
  {
  case 3:
    return LOW;
    break;
  default:
    return HIGH;
  }
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

// The CV Output circuit inverts the signal, so invert it here
void cv_out(uint16_t val)
{
  // Serial.println("CV Out: " + String(val));
  if (settings_is_quantized())
    val &= 0xFFF8;
  cv_val = val;
  write_dac(DAC_FS - cv_val);
}

void set_adj()
{
  noInterrupts();
  // critical, time-sensitive code here
  if (user_adjusting())
  {
    // Serial.println("Setting new " + String(settings_get_adj_fxn()) + ": " + String(adj));
    switch (settings_get_adj_fxn())
    {
    case 0:
      scale_adj = adj;
      set_scale();
      break;
    case 1:
      offset_adj = adj;
      set_scale();
      break;
    case 2:
      cv_out(adj >> 2); // unscaled
      break;
    }
  }
  interrupts();
}

boolean event_pending = false;
void do_delay(unsigned int d)
{
  // Serial.println("");
  // Serial.println("Do delay: " + String(d) + " millis: " + String(millis()));
  if (d < 200)
  {
    delay(d);
    // Serial.println("Delay done at: " + String(millis()));
  }
  else
  {
    boolean ser_avail, adjusting;
    unsigned long targetMillis = millis() + d;
    do
    {
      adjusting = e.is_adjusting();
      ser_avail = false && Serial.available() > 0;
      event_pending = adjusting || ser_avail || keypress;
      // Serial.println("millis: " + String(millis()));
    } while (millis() < targetMillis && !event_pending && triggered);
  }
}

unsigned int calc_delay(unsigned int d, int rnd)
{
  float delay_adj = rnd * d / 200.0;
  // reuse rnd here
  rnd = random(int(-delay_adj), int(delay_adj));
  return d + rnd;
}

void do_pulse(unsigned int on_time, unsigned int off_time, int aval1, int aval2)
{
  cv_out(aval1);
  gate.set();
  do_delay(on_time);
  gate.reset();
  cv_out(aval2);
  do_delay(off_time);
}

void send_one_pulse(float the_delay, float longest_pulse, float shortest_pulse, int rnd, float the_swell)
{
  unsigned int my_delay = calc_delay(the_delay, rnd);
  int span = longest_pulse - shortest_pulse;
  int normalized_delay = my_delay - shortest_pulse;
  int aval1 = max(0, normalized_delay * DAC_FS / span);
  int aval2 = max(0, (normalized_delay + the_swell / 2) * DAC_FS / span);
  scale_and_offset(&aval1);
  scale_and_offset(&aval2);
  do_pulse(my_delay, my_delay, aval1, aval2);
}