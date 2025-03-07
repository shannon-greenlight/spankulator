enum
{
  SETTINGS_VER = 0,
  SETTINGS_POT_FXN,
  SETTINGS_DC_FXN,
  SETTINGS_QUANTIZE,
  SETTINGS_CLK,
  SETTINGS_EXT_TRIG,
  SETTINGS_ENCODER,
  SETTINGS_WIFI,
  SETTINGS_USB_DIRECT,
  SETTINGS_SCREEN,
  SETTINGS_NAME,
  SETTINGS_RESET,
  SETTINGS_CAL,
  SETTINGS_BOARD_GENERATION,
  SETTINGS_NUM_PARAMS,
};

enum
{
  SETTING_CV_OFF = 0,
  SETTING_CV_SCALE,
  SETTING_CV_OFFSET,
  SETTING_CV_VALUE,
};

uint16_t _settings_params[SETTINGS_NUM_PARAMS];
uint16_t _settings_mins[] = {0, SETTING_CV_OFF, SETTING_CV_OFF, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1};
uint16_t _settings_maxs[] = {0, SETTING_CV_VALUE, SETTING_CV_VALUE, 1, 1, 1, 1, 2, 1, 9999, 0, 0, 1, BOARD_GENERATION};
uint16_t _settings_init_vals[] = {0, SETTING_CV_OFF, SETTING_CV_OFF, 1, 0, 0, 0, 1, 0, 15, 0, 0, 0, BOARD_GENERATION};
uint16_t *settings_stuff[] = {_settings_params, _settings_mins, _settings_maxs, _settings_init_vals};
String settings_labels[] = {"Version: ", "CV Pot: ", "Sig In: ", "Quantize: ", "Clock: ", "Ext Trig: ", "Encoder Type: ", "WiFi: ", "USB Direct: ", "Screen Saver: ", "Name: ", "Reset: ", "Calibrate: ", "Board Generation: "};
String settings_string_params[] = {VERSION_NUM, "Off,Scale ,Offset,Value ", "Off,Scale ,Offset,Value ", "No ,Yes", "Internal,External", "Enabled ,Disabled", "Normal ,Reverse", "Disabled,Enabled ,Reset", "Disabled,Enabled ", "", "$~", "Push Activate", "ADC,DAC", ""};
EEPROM_String settings_device_name(20);
EEPROM_String settings_string_vars[] = {settings_device_name};
Greenface_gadget settings_spanker("Settings", settings_labels, settings_stuff, sizeof(_settings_params) / sizeof(_settings_params[0]));
bool settings_param_active[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0};

void settings_fxn()
{
  adc_cal_screen = 0;
  dac_cal_screen = 0;
  selected_fxn = &settings_spanker;
  settings_spanker.display();
  disable_ext_trigger();
  trigger_fxn = noop;
  trigger_control.triggered = false;
}

bool is_usb_direct()
{
  return settings_spanker.get_param(SETTINGS_USB_DIRECT) ? true : false;
}

void set_usb_direct()
{
  ui.terminal_mirror = !is_usb_direct();
}

void settings_put_usb_direct(int val)
{
  settings_spanker.param_put(val, SETTINGS_USB_DIRECT);
  set_usb_direct();
}

uint16_t settings_get_pot_fxn()
{
  return settings_spanker.get_param(SETTINGS_POT_FXN);
}

uint16_t settings_get_dc_fxn()
{
  return settings_spanker.get_param(SETTINGS_DC_FXN);
}

boolean settings_is_quantized()
{
  return settings_spanker.get_param(SETTINGS_QUANTIZE) == 1;
}

void settings_put_quantized(uint16_t val)
{
  // settings_spanker.put_param_num(SETTINGS_QUANTIZE);
  settings_spanker.param_put(val, SETTINGS_QUANTIZE);
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
  // settings_spanker.put_param_num(SETTINGS_CLK);
  settings_spanker.param_put(val, SETTINGS_CLK);
  if (settings_is_ext_clk())
  {
    set_repeat_on(false);
  }
  // set_repeat_on(!settings_is_ext_clk());
}

void settings_toggle_ext_clk()
{
  settings_spanker.put_param_num(SETTINGS_CLK);
  settings_spanker.put_param(settings_is_ext_clk() ? 0 : 1);
  set_repeat_on(!settings_is_ext_clk());
}

uint16_t settings_get_ext_trig()
{
  return settings_spanker.get_param(SETTINGS_EXT_TRIG);
}

void settings_put_ext_trig(uint16_t val)
{
  // settings_spanker.put_param_num(SETTINGS_EXT_TRIG);
  settings_spanker.param_put(val, SETTINGS_EXT_TRIG);
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

uint16_t settings_get_wifi()
{
  return settings_spanker.get_param(SETTINGS_WIFI);
}

bool usb_direct_enabled()
{
  return settings_spanker.get_param(SETTINGS_USB_DIRECT);
}

int settings_get_inactivity_timeout()
{
  return settings_spanker.get_param(SETTINGS_SCREEN);
}

int settings_get_board_generation()
{
  return settings_spanker.get_param(SETTINGS_BOARD_GENERATION);
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
  case SETTINGS_USB_DIRECT:
    set_usb_direct();
    break;
  case SETTINGS_WIFI:
    // mode = settings_fxn.get_param(SETTINGS_WIFI);
    // if (settings_spanker.get_param(SETTINGS_WIFI) == 2)
    // {
    //   select_wifi_screen = 0;
    //   // system_message = "WiFi Reset";
    // }
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
  case SETTINGS_USB_DIRECT:
    set_usb_direct();
    break;
  case SETTINGS_WIFI:
    // mode = settings_fxn.get_param(SETTINGS_WIFI);
    // if (settings_spanker.get_param(SETTINGS_WIFI) == 2)
    // {
    //   select_wifi_screen = 0;
    //   // system_message = "WiFi Reset";
    // }
    break;
  }
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
  case SETTINGS_WIFI:
    if (selected_fxn == &settings_spanker)
    {
      // Serial.println("Trigger -> Enter wifi " + selected_fxn->name);
      enter_wifi();
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
  settings_spanker.active_params = settings_param_active;
  settings_set_ext_trig(); // make sure hardware reflects variable
}
