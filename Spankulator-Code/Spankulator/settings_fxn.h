uint16_t _settings_params[11];
uint16_t _settings_mins[] = {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0};
uint16_t _settings_maxs[] = {0, 2, 1, 1, 1, 1, 1, 9999, 0, 0, 1};
uint16_t _settings_init_vals[] = {0, 0, 1, 0, 0, 0, 1, 15, 0, 0, 0};
uint16_t *settings_stuff[] = {_settings_params, _settings_mins, _settings_maxs, _settings_init_vals};
String settings_labels[] = {"Version: ", "Adj Fxn: ", "Quantize: ", "Clock: ", "Ext Trig: ", "Encoder Type: ", "WiFi: ", "Screen Saver: ", "Name: ", "Reset: ", "Calibrate: "};
String settings_string_params[] = {VERSION_NUM, "Scale ,Offset,Value ", "No ,Yes", "Internal,External", "Enabled ,Disabled", "Normal ,Reverse", "Disabled,Enabled ", "", "$~", "Push Trig", "ADC,DAC"};
EEPROM_String settings_device_name(20);
EEPROM_String settings_string_vars[] = {settings_device_name};
Greenface_gadget settings_spanker("Settings", settings_labels, settings_stuff, sizeof(_settings_params) / sizeof(_settings_params[0]));

#define SETTINGS_VER 0
#define SETTINGS_ADJ_FXN 1
#define SETTINGS_QUANTIZE 2
#define SETTINGS_CLK 3
#define SETTINGS_EXT_TRIG 4
#define SETTINGS_ENCODER 5
#define SETTINGS_WIFI 6
#define SETTINGS_SCREEN 7
#define SETTINGS_NAME 8
#define SETTINGS_RESET 9
#define SETTINGS_CAL 10

void settings_fxn()
{
  adc_cal_screen = 0;
  dac_cal_screen = 0;
  selected_fxn = &settings_spanker;
  settings_spanker.display();
  disable_ext_trigger();
}

void settings_put_param(int val)
{
  settings_spanker.put_param(val);
  switch (settings_spanker.param_num)
  {
  case SETTINGS_ENCODER:
    set_encoder();
    break;
  case SETTINGS_SCREEN:
    restore_display();
    break;
  }
}

void settings_adjust_param(int encoder_val, unsigned long delta)
{
  settings_spanker.adjust_param(encoder_val, delta);
  switch (settings_spanker.param_num)
  {
  case SETTINGS_ENCODER:
    set_encoder();
    break;
  case SETTINGS_SCREEN:
    restore_display();
    break;
  }
}

uint16_t settings_get_adj_fxn()
{
  return settings_spanker.get_param(SETTINGS_ADJ_FXN);
}

boolean settings_is_quantized()
{
  return settings_spanker.get_param(SETTINGS_QUANTIZE) == 1;
}

void settings_put_quantized(uint16_t val)
{
  settings_spanker.put_param_num(SETTINGS_QUANTIZE);
  settings_spanker.put_param(val);
}

String settings_get_device_name()
{
  return settings_spanker.get_param_as_string_var(SETTINGS_NAME);
}

boolean settings_is_ext_clk()
{
  return settings_spanker.get_param(SETTINGS_CLK) == 1;
}

void settings_put_ext_clk(uint16_t val)
{
  settings_spanker.put_param_num(SETTINGS_CLK);
  settings_spanker.put_param(val);
}

void settings_toggle_ext_clk()
{
  settings_spanker.put_param_num(SETTINGS_CLK);
  settings_spanker.put_param(settings_is_ext_clk() ? 0 : 1);
}

uint16_t settings_get_ext_trig()
{
  return settings_spanker.get_param(SETTINGS_EXT_TRIG);
}

void settings_put_ext_trig(uint16_t val)
{
  settings_spanker.put_param_num(SETTINGS_EXT_TRIG);
  settings_spanker.put_param(val);
  put_disable_ext_trigger(val);
}

void settings_toggle_ext_trig()
{
  settings_put_ext_trig(!settings_get_ext_trig());
}

void settings_set_ext_trig()
{
  int ext_trig = settings_spanker.get_param(SETTINGS_EXT_TRIG);
  put_disable_ext_trigger(ext_trig);
}

uint16_t settings_get_encoder_type()
{
  return settings_spanker.get_param(SETTINGS_ENCODER);
}

bool wifi_enabled()
{
  return settings_spanker.get_param(SETTINGS_WIFI);
}

int settings_get_inactivity_timeout()
{
  return settings_spanker.get_param(SETTINGS_SCREEN);
  // return settings_spanker.get_param(SETTINGS_SCREEN) * HEART_RATE * 60;
  // return max(300, settings_spanker.get_param(SETTINGS_SCREEN) * 300); // 1 minute = 300 heartbeats
}

void settings_trigger()
{
  switch (settings_spanker.param_num)
  {
  case SETTINGS_RESET:
    init_all();
    break;

  case SETTINGS_CAL:
    switch (settings_spanker.get_param(SETTINGS_CAL))
    {
    case 0:
      adc_cal();
      break;
    case 1:
      dac_cal();
      break;
    }
    break;

  default:
    ui.terminal_debug("Initialized: " + String(eeprom_is_initialized()) + " Pattern: " + String(EEPROM_INIT_PATTERN));
    break;
  }
}

void settings_init()
{
  settings_spanker.string_vars[0].put("Spanky");
}

void settings_begin()
{
  settings_spanker.begin();
  settings_spanker.string_vars = settings_string_vars;
  settings_spanker.string_vars[0].begin(false);
  settings_spanker.string_vars[0].xfer();
  settings_spanker.string_params = settings_string_params;
  settings_spanker.trigger_fxn = settings_trigger;
  settings_set_ext_trig(); // make sure hardware reflects variable
}
