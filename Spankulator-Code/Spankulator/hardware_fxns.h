// hardware
#include <BufferedOutput.h>
const byte toggle_out_pin = 5;
const byte repeat_led_pin = 6;
const byte gate_out_pin = 9;
const byte disable_trigger_pin = 11;
const byte triggered_led_pin = 12;
const byte disable_ext_trigger_pin = 13;

const byte aout_pin = A0;
const byte ain1_pin = A1;
const byte ain2_pin = A2;
const byte ain3_pin = A3;

// buttons
const byte up_button_pin = 0; // fxn +
const byte dn_button_pin = 1; // fxn -
const byte repeat_button_pin = 4;
const byte trigger_button_pin = 7;
const byte param_up_button_pin = A7; // ^
const byte param_dn_button_pin = A6; // v
const byte left_button_pin = 10;     // <
const byte right_button_pin = 8;     // >

BufferedOutput gate(gate_out_pin);
BufferedOutput tog(toggle_out_pin);

// Rotary Encoder
void intFxnA(void)
{
    e.aChanInt();
}

void intFxnB(void)
{
    e.bChanInt();
}

void hardware_begin()
{
    pinMode(repeat_led_pin, OUTPUT);
    pinMode(disable_trigger_pin, OUTPUT);
    pinMode(disable_ext_trigger_pin, OUTPUT);
    pinMode(triggered_led_pin, OUTPUT);
    pinMode(0, INPUT);
    pinMode(1, INPUT);

    pinMode(up_button_pin, INPUT_PULLUP);
    pinMode(dn_button_pin, INPUT_PULLUP);
    pinMode(repeat_button_pin, INPUT_PULLUP);
    pinMode(trigger_button_pin, INPUT_PULLUP);
    pinMode(param_up_button_pin, INPUT_PULLUP);
    pinMode(param_dn_button_pin, INPUT_PULLUP);
    pinMode(right_button_pin, INPUT_PULLUP);
    pinMode(left_button_pin, INPUT_PULLUP);

    // adc_config_hardware();

    pinMode(param_dn_button_pin, INPUT_PULLUP);

    // analogReadCorrection(-23, 2000);
    // analogReadCorrection(-30, 2047);

    analogWriteResolution(10);
    // DAC->CTRLB.reg &= 0x3f; // use EXT as the reference
    // DAC->CTRLB.reg |= 0x80; // use EXT as the reference
    analogWrite(A0, 511);
}
