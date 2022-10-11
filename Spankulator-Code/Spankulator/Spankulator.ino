// Spankulator main
typedef void (*FunctionPointer)();

// Load 3rd party libraries
#include <stdlib.h>
#include <Wire.h> // For I2C comm, but needed for not getting compile error
#include <Timer5.h>
#include <Adafruit_FRAM_I2C.h>
#include <WiFiNINA.h>
#include <arduino_secrets.h>

#define BOARD_GENERATION 4
#include "version_num.h"
#include "hardware_defs.h"

void adc_config_hardware();

// settings functions
uint16_t settings_get_adj_fxn();
boolean settings_is_quantized();
String settings_get_device_name();
boolean settings_is_ext_clk();
uint16_t settings_get_ext_trig();
void settings_put_ext_trig(uint16_t);
void settings_put_quantized(uint16_t);
void set_encoder();
bool wifi_enabled(void);

// hilevel functions
void new_fxn();
// void reset_inactivity_timer();
// void check_inactivity_timer();
void restore_display();
void check_display_restore();

// variables
String version_num = VERSION_NUM;
volatile int keypress = 0;               // set by several sources
volatile boolean fp_key_pressed = false; // only set by front panel keypress
volatile int inactivity_timer = 0;
boolean remote_adjusting = false;
boolean esc_mode = false;
boolean triggered = false;
boolean doing_trigger = false;
String in_str = ""; // for serial input
boolean cmd_available = false;
String wifi_ui_message = "";

uint16_t cv_val;
int adj; // from pot
int scale_adj = ADC_FS;
int offset_adj = ADC_MID;
float scale;
int offset;

// Load Greenface libraries
#include <TerminalVT100.h>
#include <RotaryEncoder.h>
#include <Greenface_EEPROM.h>
#include <EEPROM_fxns.h>
#include <Greenface_gadget.h>
#include <WIFI_Util.h>

#include "hardware_fxns.h"
#include "general_fxns.h"

#include "SPANK_ui.h"
#include "face1.h" // greenface logo art

Greenface_gadget *selected_fxn;
EEPROM_Int fxn = EEPROM_Int(0, 1000);  // set max to real max when num_fxns has been defined
EEPROM_Bool repeat_on = EEPROM_Bool(); // sizeof fxn val

#define UP_FXN 0
#define DN_FXN 1
#define STRETCH_FXN 2
#define TOGGLE_FXN 3
#define MAYTAG_FXN 4
#define LFO_FXN 5
#define USER_FXN 6
#define DVM_FXN 7
#define WIFI_FXN 8
#define SETTINGS_FXN 9

#include <Greenface_wifi_fxns.h>
#include "up_spanker.h"
#include "dn_spanker.h"
#include "stretch_spanker.h"
#include "toggle_spanker.h"
#include "maytag_spanker.h"

#include "lfo.h"
#include "user_fxn.h"
#include "adc.h"
#include "dac.h"
#include "dvm_fxn.h"
#include "wifi_ui.h"
#include "settings_fxn.h"
// #include "wdt.h"

#include "hilevel_fxns.h"
void terminal_print_status()
{
  if (ui.terminal_mirror)
  {
    ui.t.setCursor(STATUS_ROW, "33");
    ui.t.print(triggered ? "TRIGGERED" : "         ");
    String rpeat = repeat_on.get() ? "ON " : "OFF";
    ui.t.setCursor(FXN_ROW, "16");
    ui.t.print("Repeat: " + rpeat);
    ui.t.setCursor(FXN_ROW, "28");
    float percent = 100 * scale;
    ui.t.print("CV Scale: " + String(percent) + String("%"));
    ui.t.print(" Offset: " + String(offset));
  }
}

// EEO must come after last EEPROM var is declared
#define EEO Greenface_EEPROM::eeprom_offset

void setup(void)
{
  begin_all();
  ee_info.begin(false);
  ee_info.xfer();
  delay(500);

  offset_adj = ADC_MID;
  set_adj();
  scale = 1.0;
  offset = 0;

  set_encoder(); // sets msb,lsb for two types of encoder

  if (!eeprom_is_initialized() || !digitalRead(up_button_pin))
  {
    init_all();
    wifi_password.init();
    wifi_password.put("");
    wifi_ssid.init();
    wifi_ssid.put("");
  }

  // connect if wifi is active
  wifi_attempt_connect(false);

  ui.splash();
  delay(2500);
  exe_fxn();
}

void loop()
{
  // ui.terminal_debug("inactivity_timer: " + String(inactivity_timer) + " keypress: " + String(keypress));

  // check_serial affects
  // instr and cmd_avail for process_cmd
  // and keypress used directly below
  check_serial();

  // terminal_print_status();

  if (keypress || cmd_available)
  {
    if (cmd_available)
    {
      process_cmd(in_str);
      in_str = "";
      cmd_available = false;
    }
    else
    {
      check_display_restore();
      process_keypress();
    }
  }
  else
  {
    set_adj();
    housekeep();
    do_server();
  }

  check_trigger();
  check_rotary_encoder();
  ui.check_inactivity_timer(settings_get_inactivity_timeout());
}
