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

// 30 beats per second
#define HEART_RATE 30

#define ADC_PIN A2

#define ADC_BITS 12
//#define ADC_FS (1 << ADC_BITS) - 1
#define ADC_FS 4095
//#define ADC_MID (1 << ADC_BITS - 1)
#define ADC_MID 2048
#define ADC_SCALE 409.6

#define ADC_READS_SHIFT 8
#define ADC_READS_COUNT (1 << ADC_READS_SHIFT)

#define ADC_MIN_GAIN 0x0400
#define ADC_UNITY_GAIN 0x0800
#define ADC_MAX_GAIN (0x1000 - 1)
#define ADC_RESOLUTION_BITS 12
#define ADC_RANGE (1 << ADC_RESOLUTION_BITS)
#define ADC_TOP_VALUE (ADC_RANGE - 1)

#define MAX_TOP_VALUE_READS 10

#define DAC_BITS 10
//#define DAC_FS (1 << DAC_BITS)
// #define DAC_FS 1023
//#define DAC_MID (1 << DAC_BITS - 1)
#define DAC_MID 512

//#define ADC_DAC_SHIFT (ADC_BITS - DAC_BITS)
#define ADC_DAC_SHIFT 2

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
#include "version_num.h"
#include "TerminalVT100.h"
#include "RotaryEncoder.h"
#include "Greenface_EEPROM.h"
#include "EEPROM_Arr16.h"
#include "EEPROM_Int.h"
#include "EEPROM_Bool.h"
#include "EEPROM_String.h"

// Rotary Encoder
void intFxnA(void)
{
  e.aChanInt();
}

void intFxnB(void)
{
  e.bChanInt();
}

#include "hardware_defs.h"
#include "hardware_fxns.h"
#include "general_fxns.h"

#include "GREENFACE_ui.h"
#include "SPANK_ui.h"
#include "WIFI_Util.h"
#include "SPANK_fxn.h"
SPANK_fxn *selected_fxn;
#include "wifi_fxns.h"

#include "face1.h" // greenface logo art

#define EEPROM_DATA_START 16
#define EEPROM_INIT_FLAG 14
#define EEPROM_OFFSET_FLAG 12
#define EEPROM_INIT_PATTERN 0x55aa

int Greenface_EEPROM::eeprom_offset = EEPROM_DATA_START;
EEPROM_Int fxn = EEPROM_Int(0, 1000);  // set max to real max when num_fxns has been defined
EEPROM_Bool repeat_on = EEPROM_Bool(); // sizeof fxn val

// todo should check for version change or new eeprom usage
bool is_initialized()
{
  return fxn.read_int(EEPROM_INIT_FLAG) == EEPROM_INIT_PATTERN && fxn.read_int(EEPROM_OFFSET_FLAG) == Greenface_EEPROM::eeprom_offset;
}

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
    // ui.t.print(" Keys: " + String(all_buttons_up()));
  }
}

// EEO must come after last EEPROM var is declared
#define EEO Greenface_EEPROM::eeprom_offset

void setup(void)
{
  begin_all();
  delay(500);

  offset_adj = ADC_MID;
  set_adj();
  scale = 1.0;
  offset = 0;

  set_encoder(); // sets msb,lsb for two types of encoder

  if (!is_initialized())
  {
    init_all();
  }

  // connect if wifi is active
  wifi_attempt_connect(false);

  ui.splash();
  // Serial.println("EEO: " + String(EEO));
  delay(2500);
  exe_fxn();

  // analogReadResolution(8);
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
