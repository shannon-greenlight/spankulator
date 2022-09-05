#include <Wire.h> // For I2C comm, but needed for not getting compile error
#include <TerminalVT100.h>
#include <stdlib.h>

#define LFO_BASE_PERIOD 2
#define LFO_BASE_DELAY 2
#define LFO_PARTS 128
#define LFO_PERIOD 0
#define LFO_WAVEFORM 1
#define LFO_RANDOMNESS 2

void lfo_fxn();

uint16_t wave_table[128] PROGMEM;
int delay_usecs = 39047;
boolean lfo_params_set = false;
// for ext clk
uint16_t lfo_index = 0;

uint16_t _lfo_params[3];
uint16_t _lfo_mins[] = {2, 0, 0};
uint16_t _lfo_maxs[] = {32767, 3, 99};
uint16_t _lfo_init_vals[] = {100, 0, 0};
uint16_t *lfo_stuff[] = {_lfo_params, _lfo_mins, _lfo_maxs, _lfo_init_vals};
String lfo_string_params[] = {"", "Sine,Inv Sine,Ramp Up,Ramp Down", ""};
String lfo_labels[] = {"Period: ", "Waveform: ", "Randomness: "};
SPANK_fxn lfo_spanker("LFO", lfo_labels, lfo_stuff, sizeof(_lfo_params) / sizeof(_lfo_params[0]), &ui);

// void write_dac(uint16_t val) {
//   val = ( val << 2 ) | 0x3000;
//   digitalWrite(dac_select_pin, LOW); //initialize the chip
//   SPI.transfer16(val); // write to device
//   digitalWrite(dac_select_pin, HIGH); // de-initialize the chip
// }

String lfo_waveform_name()
{
    return lfo_string_params[selected_fxn->get_param(LFO_WAVEFORM)];
    // return lfo_waveform_num.get()==0 ? "Sine" : "Ramp";
}

void lfo_set_waveform()
{
    int temp;
    wifi_ui_message = "";
    const String err = F("LFO set waveform unknown table: ");
    for (int i = 0; i < LFO_PARTS; i++)
    {
        switch (selected_fxn->get_param(LFO_WAVEFORM))
        {
        case 0:
            temp = (sin(6.28 * i / LFO_PARTS) * 511) + 512;
            break;
        case 1:
            temp = (sin(6.28 * i / LFO_PARTS) * 511) + 512;
            temp = DAC_FS - temp;
            break;
        case 2:
            temp = i * DAC_FS / LFO_PARTS;
            break;
        case 3:
            temp = DAC_FS - (i * DAC_FS / LFO_PARTS);
            break;
        default:
            Serial.println(err + String(selected_fxn->get_param(LFO_WAVEFORM)));
        }
        scale_and_offset(&temp);
        int rnd = random(-selected_fxn->get_param(LFO_RANDOMNESS), selected_fxn->get_param(LFO_RANDOMNESS));
        temp += rnd * scale * DAC_FS / 100;
        temp = max(0, temp);
        temp = min(DAC_FS, temp);
        wave_table[i] = temp;
        if (i > 0)
            wifi_ui_message += ", ";
        wifi_ui_message += String(wave_table[i]);
    }

    // ui.terminal_debug("Scale: " + String(scale) + " Offset: " + String(offset));
}

String lfo_params_toJSON()
{
    lfo_set_waveform();
    return selected_fxn->params_toJSON();
}

void lfo_housekeep()
{
    if (user_adjusting() || remote_adjusting)
    {
        remote_adjusting = false;
        lfo_set_waveform();
        int data[128];

        ui.fill(BLACK, 16);
        //   ui.printText("LFO " + String(lfo_period.get()) + "ms",0,0,2);
        // todo: move this calc to ui.graphData
        for (int i = 0; i < 128; i++)
        {
            float val = float(wave_table[i]) / DAC_FS;
            data[i] = max(16, 63 - (int(48 * val)));
        }
        ui.graphData(data);
        ui.showDisplay();
    }
}

//#define LFO_FREQ_FACTOR 993.982
#define LFO_FREQ_FACTOR 1000.0

void lfo_set_period()
{
    delay_usecs = int(1000.0 * (selected_fxn->get_param(LFO_PERIOD)) / LFO_PARTS) - 6;
}

void lfo_put_param(uint16_t val)
{
    selected_fxn->put_param(val);
    switch (selected_fxn->param_num)
    {
    case LFO_PERIOD:
        lfo_set_period();
        break;
    case LFO_WAVEFORM:
    case LFO_RANDOMNESS:
        lfo_set_waveform();
        break;
    }
}

void lfo_adjust_param(int e, unsigned long delta)
{
    selected_fxn->adjust_param(e, delta);
    lfo_put_param(selected_fxn->get_param());
}

void lfo_set_params()
{
    lfo_set_period();
    lfo_set_waveform();
}

void lfo_do_trigger()
{
    if (!lfo_params_set)
    {
        lfo_set_params();
        lfo_params_set = true;
        lfo_index = 0;
    }
    if (!settings_is_ext_clk())
    {
        gate.set();
        for (int i = 0; (i < LFO_PARTS) && !keypress && !e.getEncoderValue(); i++)
        {
            cv_out(wave_table[i]);
            delayMicroseconds(delay_usecs);
        }
        do_toggle();
        doing_trigger = false;
        gate.reset();
    }
    else
    {
        if (lfo_index == 0)
        {
            gate.set();
        }
        cv_out(wave_table[lfo_index++]);
        if (lfo_index >= LFO_PARTS)
        {
            do_toggle();
            doing_trigger = false;
            gate.reset();
            lfo_index = 0;
        }
    }
}

void lfo_fxn()
{
    selected_fxn = &lfo_spanker;
    lfo_set_params();
    lfo_set_waveform();
    lfo_params_set = false;
    lfo_index = 0;
    lfo_housekeep();
    ui.newFxn(selected_fxn->name);
    selected_fxn->printParams();
}

void lfo_begin()
{
    // Serial.println("LFO beginning");
    lfo_spanker.begin();
    lfo_spanker.trigger_fxn = lfo_do_trigger;
    lfo_spanker.string_params = lfo_string_params;
}
