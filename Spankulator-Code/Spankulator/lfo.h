#include <Wire.h> // For I2C comm, but needed for not getting compile error
#include <TerminalVT100.h>
#include <stdlib.h>

#define LFO_PARTS 128

enum
{
    LFO_PERIOD = 0,
    LFO_WAVEFORM,
    LFO_RANDOMNESS,
    LFO_MIN_PARTS,
    LFO_NUM_PARAMS,
};

void lfo_fxn();

uint16_t wave_table[LFO_PARTS] PROGMEM;
boolean lfo_params_set = false;
// for ext clk
uint16_t lfo_index = 0;

uint16_t _lfo_params[LFO_NUM_PARAMS];
uint16_t _lfo_mins[] = {2, 0, 0, 4};
uint16_t _lfo_maxs[] = {32767, 3, 99, 128};
uint16_t _lfo_init_vals[] = {100, 0, 0, 16};
uint16_t *lfo_stuff[] = {_lfo_params, _lfo_mins, _lfo_maxs, _lfo_init_vals};
String lfo_string_params[] = {"", "Sine,Inv Sine,Ramp Up,Ramp Down", "", ""};
String lfo_labels[] = {"Period: ", "Waveform: ", "Randomness: ", "Min Parts: "};
Greenface_gadget lfo_spanker("LFO", lfo_labels, lfo_stuff, sizeof(_lfo_params) / sizeof(_lfo_params[0]));

unsigned int sample_time; // in msecs
unsigned int sample_skip; // how many samples in table to skip when generating signal

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
        int rnd = random(-selected_fxn->get_param(LFO_RANDOMNESS), selected_fxn->get_param(LFO_RANDOMNESS));
        temp += rnd * scale * DAC_FS / 100;
        temp = constrain(temp, 0, DAC_FS);
        wave_table[i] = temp;
        scale_and_offset(&wave_table[i]);
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
        int data[LFO_PARTS];

        ui.fill(BLACK, 16);
        // ui.printText("Adj LFO " + String(selected_fxn->get_param(LFO_PERIOD)) + "ms", 0, 0, 2);
        // todo: move this calc to ui.graphData
        for (int i = 0; i < LFO_PARTS; i++)
        {
            float val = float(wave_table[i]) / DAC_FS;
            data[i] = max(16, 63 - (int(48 * val)));
        }
        ui.graphData(data);
        ui.showDisplay();
    }
}

// #define LFO_FREQ_FACTOR 993.982
#define LFO_FREQ_FACTOR 1000.0

void lfo_set_period()
{
    float target_ratio = selected_fxn->get_param(LFO_PERIOD) / float(LFO_PARTS); // waveform parts * sample time
    float minimum = 100000.0;
    unsigned int max_skip = LFO_PARTS / selected_fxn->get_param(LFO_MIN_PARTS);
    unsigned int chosen_skip;
    unsigned int chosen_n;
    for (int skip = 1; skip <= max_skip; skip++)
    {
        int n = target_ratio * skip;
        float r = float(n) / skip;
        // Serial.println("R: " + String(r) + " n: " + String(n));
        if (abs(target_ratio - r) < minimum)
        {
            chosen_skip = skip;
            chosen_n = n;
            minimum = abs(target_ratio - r);
        }
    }
    sample_skip = chosen_skip;
    sample_time = chosen_n;
    // Serial.println("Ratio: " + String(target_ratio) + " Minimum: " + String(minimum));
    // Serial.println("Chosen Skip: " + String(chosen_skip) + " n: " + String(chosen_n));
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

void lfo_trigger()
{
    // gate.toggle();
    unsigned long ms = millis();
    if (trigger_go(ms))
    {
        if (settings_is_ext_clk())
        {
            is_triggered = false;
        }
        cv_out(wave_table[trigger_control.pulse_count]);
        trigger_control.next_time += sample_time;
        trigger_control.pulse_count += sample_skip;
        if (trigger_control.pulse_count >= LFO_PARTS)
        {
            trigger_control.pulse_count = 0;
        }
        if (trigger_control.pulse_count == 0)
        {
            reset_trigger();
        }
    }
}

void set_lfo_trigger()
{
    trigger_control.next_time = millis() + 1;
    trigger_control.pulse_count = 0;
    trigger_control.triggered = true;
    trigger_fxn = lfo_trigger;
    // Serial.println("Set LFO Trigger: " + String(spank_engine.delay));
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
    // MyTimer5.begin(964); // beats per sec
    MyTimer5.begin(1000); // beats per sec
    trigger_control.triggered = false;
    trigger_fxn = lfo_trigger;
}

void lfo_update_resolution()
{
    lfo_set_period();
}

update_fxn lfo_update_fxns[LFO_NUM_PARAMS] = {
    nullptr,
    nullptr,
    nullptr,
    lfo_update_resolution,
};

void lfo_begin()
{
    // Serial.println("LFO beginning");
    lfo_spanker.begin();
    lfo_spanker.trigger_fxn = set_lfo_trigger;
    lfo_spanker.string_params = lfo_string_params;
    lfo_spanker.update_fxns = lfo_update_fxns;
}
