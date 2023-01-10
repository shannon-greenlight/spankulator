// hardware
#include <BufferedOutput.h>
const byte toggle_out_pin = 5;
const byte repeat_led_pin = 6;
const byte gate_out_pin = 9;
const byte disable_trigger_pin = 11;
const byte triggered_led_pin = 12;
const byte disable_ext_trigger_pin = 13;

const byte aout_pin = A0;
// const byte ain1_pin = A1;
// const byte ain2_pin = A2;
// const byte ain3_pin = A3;

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

// adc
typedef enum _adc_chan
{
    ADC_POT = A1,
    ADC_DC = A2,
    ADC_AC = A3,
} adc_chan;

int readADC(adc_chan chan)
{
    if (chan == ADC_POT)
    {
        return analogRead(chan);
    }
    else
    {
        return ADC_FS - analogRead(chan);
    }
}

// Rotary Encoder
void intFxnA(void)
{
    e.aChanInt();
}

void intFxnB(void)
{
    e.bChanInt();
}

// DAC
// #define DAC_CTRLB_REFSEL_Pos _UINT8_(6)                                                           /* (DAC_CTRLB) Reference Selection Position */
// #define DAC_CTRLB_REFSEL_Msk (_UINT8_(0x3) << DAC_CTRLB_REFSEL_Pos)                               /* (DAC_CTRLB) Reference Selection Mask */
// #define DAC_CTRLB_REFSEL(value) (DAC_CTRLB_REFSEL_Msk & (_UINT8_(value) << DAC_CTRLB_REFSEL_Pos)) /* Assigment of value for REFSEL in the DAC_CTRLB register */
// #define DAC_CTRLB_REFSEL_INT1V_Val _UINT8_(0x0)                                                   /* (DAC_CTRLB) Internal 1.0V reference  */
// #define DAC_CTRLB_REFSEL_AVCC_Val _UINT8_(0x1)                                                    /* (DAC_CTRLB) AVCC  */
// #define DAC_CTRLB_REFSEL_VREFP_Val _UINT8_(0x2)                                                   /* (DAC_CTRLB) External reference  */
// #define DAC_CTRLB_REFSEL_INT1V (DAC_CTRLB_REFSEL_INT1V_Val << DAC_CTRLB_REFSEL_Pos)               /* (DAC_CTRLB) Internal 1.0V reference Position  */
// #define DAC_CTRLB_REFSEL_AVCC (DAC_CTRLB_REFSEL_AVCC_Val << DAC_CTRLB_REFSEL_Pos)                 /* (DAC_CTRLB) AVCC Position  */
// #define DAC_CTRLB_REFSEL_VREFP (DAC_CTRLB_REFSEL_VREFP_Val << DAC_CTRLB_REFSEL_Pos)               /* (DAC_CTRLB) External reference Position  */

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

    // DAC_CTRLB_REFSEL(DAC_CTRLB_REFSEL_INT1V_Val);
    analogWriteResolution(10);
    // DAC->CTRLB.reg &= 0x3f; // use EXT as the reference
    // DAC->CTRLB.reg |= 0x80; // use EXT as the reference
    analogWrite(A0, 511);
}
