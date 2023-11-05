// This file implements the Bounce function

EEPROM_Int dvm_mode = EEPROM_Int(0, 3); // 0-DC 1=AC, 2=DCQ, 3=ACQ
#define DVM_MEAS_BUF 128
#define DVM_DC_MODE 0
#define DVM_AC_MODE 1
#define DVM_DCQ_MODE 2
#define DVM_ACQ_MODE 3

// DVM_SCALE is the input range in bits divided by the output range in volts
// 4096 / 10.66667
#define DVM_SCALE 384.0
// #define DVM_MID ADC_MID
#define DVM_AC_OUTPUT_SCALE 1158.35
// AC SCALE = SCALE * 2 * 1.414
#define DVM_MEAS_COUNT 200

byte dvm_ctr = 0;
int meas_buf[DVM_MEAS_BUF];
String dvm_meas = "";
const int dvm_dig_num = 0;

const String dvm_modes[4] = {"DC", "AC", "DQ", "AQ"};

String dvm_get_mode_str()
{
    return dvm_modes[dvm_mode.get()];
}

bool dvm_is_dc_mode()
{
    switch (dvm_mode.get())
    {
    case DVM_DC_MODE:
    case DVM_DCQ_MODE:
        return true;
        break;
    default:
        return false;
    }
}

bool dvm_is_quantized()
{
    switch (dvm_mode.get())
    {
    case DVM_ACQ_MODE:
    case DVM_DCQ_MODE:
        return true;
        break;
    default:
        return false;
    }
}

String dvm_get_output_str()
{
    return dvm_is_quantized() ? "quantized" : "smooth";
}

String dvm_params_toJSON()
{
    const String the_modes[4] = {"DC", "AC", "DC Quantized", "AC Quantized"};
    String out = "";
    out += "{ ";
    out += toJSON("selected", "true");
    out += ", ";
    out += toJSON("param_num", "0");
    out += ", ";
    out += toJSON("label", "Mode");
    out += ", ";
    out += toJSON("type", "radio");
    out += ", ";
    out += toJSON("values", "DC,AC,DC Quantized,AC Quantized"); // todo make this smart
    out += ", ";
    // out += toJSON("value", dvm_mode.get() ? "AC" : "DC");
    out += toJSON("value", the_modes[dvm_mode.get()]);
    // out += toJSON("value", dvm_get_mode_str());
    out += " }";
    return "[" + out + "]";
}

void dvm_inc_param_num_by(int val)
{
    // only one param, do nothing
}

void dvm_adjust_param(int val, unsigned long delta)
{
    dvm_mode.inc(val);
    int mode = dvm_mode.get();
    // Serial.print("Mode: :");
    // Serial.println(mode);
    if (mode < 0)
    {
        dvm_mode.put(3);
    }
    else
    {
        if (mode > 3)
            dvm_mode.put(0);
    }
    ui.printText("Bounce " + dvm_get_mode_str() + " ", 0, 0, 2);
    ui.printLine("      ", 20, 4); // clear measurement
    ui.printText("Output: " + dvm_get_output_str(), 0, 20, 1);
}

void dvm_put_param(int val)
{
    dvm_mode.put(val);
    dvm_adjust_param(0, 0); // prints result
}

void analog_out(int dac_val)
{
    // bypass write_dac and write directly. Quantization is local to this fxn.
    // write_dac(dac_val);
    analogWrite(aout_pin, dac_val);
    analogWrite(gate_out_pin, dac_val); // PWM proportional to dac value
}

void dvm_do_trigger()
{
    boolean doing_trigger = false;
    noInterrupts();
    byte mode = dvm_mode.get();
    adc_chan chan = dvm_is_dc_mode() ? ADC_DC : ADC_AC;
    byte buf_len = dvm_is_dc_mode() ? 32 : DVM_MEAS_BUF;
    int raw = readADC(chan);
    int reading = raw;
    float dac_temp;

    static int old_dac_val = 0;

    // Invert and correct for offset
    reading = ADC_MID - reading;
    // reading += 9; // adjust offset
    // ui.terminal_debug("Raw: " + String(raw) + " Reading: " + String(reading));
    // Serial.print("Reading: ");
    // Serial.println((reading));

    meas_buf[dvm_ctr++] = reading;

    if (dvm_ctr >= buf_len || !repeat_on.get())
    {
        char buf[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // buffer to print up to 9 digits
        // now calc RMS val
        float val = 0.0;
        uint16_t dac_val = 0;
        // Serial.println();
        for (int i = 0; i < buf_len; i++)
        {
            val += adc_scale_meas(meas_buf[i], dvm_is_dc_mode());
        }
        val = val / buf_len;

        // ui.terminal_debug("Val: " + String(val) + " Reading: " + String(raw));
        //  val *= .9653;

        if (!dvm_is_dc_mode())
        {
            val = sqrt(val);
            // dac_val = (val * DVM_SCALE * 2 * 1.414);
            dac_val = int(val * DVM_AC_OUTPUT_SCALE) >> ADC_DAC_SHIFT;
        }
        else
        {
            dac_val = (int(val * DVM_SCALE) + ADC_MID) >> ADC_DAC_SHIFT;
        }

        // dac_temp = float(dac_val - DAC_MID) * scale_adj / ADC_FS; // adjust top
        dac_temp = float(dac_val - DAC_MID); // no adjustment here!
        dac_val = int(dac_temp) + DAC_MID;
        dac_val = min(DAC_FS, dac_val);
        dac_val = max(1, dac_val);
        dac_val = DAC_FS - dac_val; // hardware inverts signal, invert it back
        if (dvm_is_quantized())
        {
            // ui.terminal_debug("dacVal: " + String(dac_val) + " Old: " + String(old_dac_val) + " Diff: " + String(dac_val - old_dac_val));
            if (abs(dac_val - old_dac_val) > 4)
            {
                dac_val += 4;
                dac_val &= 0xFFF8;
                analog_out(dac_val);

                old_dac_val = dac_val;
            }
        }
        else
        {
            scale_and_offset(&dac_val);
            analog_out(dac_val);
        }

        if (raw < 0 || raw > 5000)
        {
            dvm_meas = raw < 511 ? "+++++" : "-----";
            ui.printLine(dvm_meas, 20, 4);
        }
        else
        {
            if (!dvm_is_dc_mode())
            {
                snprintf(buf, 6, " %f.2", val);
            }
            else
            {
                snprintf(buf, 6, "%+f.2", val);
            }
            dvm_meas = String(buf);
            ui.printLine(dvm_meas, 20, 4);
            wifi_ui_message += dvm_meas;
            // send_data_to_USB(' ');
        }
        dvm_ctr = 0;
        doing_trigger = false;
        wifi_ui_message = dvm_meas + " V" + dvm_get_mode_str();
        do_toggle();
    }
    else
    {
        doing_trigger = true;
    }
    // wifi_ui_message = dvm_meas + " V" + dvm_get_mode_str();
    interrupts();
    // gate.set();
    // gate.reset();
}

volatile float dvm_val;
void dvm_dc_calc()
{
    dvm_val = read_dc_filtered(ADC_DC);
    uint16_t dac_raw = int(dvm_val) >> 2;
    scale_and_offset(&dac_raw);
    analog_out(dac_raw);
    dvm_val -= ADC_MID;
}

void dvm_ac_calc()
{
    int raw = readADC(ADC_AC);
    int reading = raw;
    float dac_temp;
    byte buf_len = DVM_MEAS_BUF;

    static int old_dac_val = 0;

    // Invert and correct for offset
    reading = ADC_MID - reading;

    meas_buf[dvm_ctr++] = reading;
    if (dvm_ctr >= buf_len || !repeat_on.get())
    {
        // calc RMS val
        float val = 0.0;
        uint16_t dac_val = 0;

        for (int i = 0; i < buf_len; i++)
        {
            val += adc_scale_meas(meas_buf[i], dvm_is_dc_mode());
        }
        val = val / buf_len;

        val = sqrt(val);
        dvm_val = val;
        // dac_val = (val * DVM_SCALE * 2 * 1.414);
        dac_val = int(val * DVM_AC_OUTPUT_SCALE) >> ADC_DAC_SHIFT;

        // dac_temp = float(dac_val - DAC_MID) * scale_adj / ADC_FS; // adjust top
        dac_temp = float(dac_val - DAC_MID); // no adjustment here!
        dac_val = int(dac_temp) + DAC_MID;
        dac_val = min(DAC_FS, dac_val);
        dac_val = max(1, dac_val);
        dac_val = DAC_FS - dac_val; // hardware inverts signal, invert it back
        if (dvm_is_quantized())
        {
            // ui.terminal_debug("dacVal: " + String(dac_val) + " Old: " + String(old_dac_val) + " Diff: " + String(dac_val - old_dac_val));
            if (abs(dac_val - old_dac_val) > 4)
            {
                dac_val += 4;
                dac_val &= 0xFFF8;
                analog_out(dac_val);

                old_dac_val = dac_val;
            }
        }
        else
        {
            scale_and_offset(&dac_val);
            analog_out(dac_val);
        }

        dvm_ctr = 0;
        trigger_control.triggered = false;
    }
    else
    {
        trigger_control.triggered = true;
    }
}

void dvm_trigger()
{
    unsigned long ms = millis();
    if (trigger_control.triggered && ms >= trigger_control.next_time)
    {
        if (dvm_is_dc_mode())
        {
            dvm_dc_calc();
        }
        else
        {
            dvm_ac_calc();
        }
        trigger_control.pulse_count--;
        trigger_control.next_time = ms + 1;
        do_cv_mod();
    }
}

void dvm_set_pulse_count()
{
    trigger_control.pulse_count = repeat_on.get() ? 10 : DVM_MEAS_COUNT;
}

void dvm_housekeep()
{
    char buf[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // buffer to print up to 9 digits
    if (trigger_control.triggered && trigger_control.pulse_count <= 0)
    {
        if (!dvm_is_dc_mode())
        {
            snprintf(buf, 6, " %f.2", dvm_val);
        }
        else
        {
            float meas = (-2.44e-3 * dvm_val); // 2.44, 5/2048  2.34 .123
            snprintf(buf, 6, "%+f.2", meas);
        }
        dvm_meas = String(buf);
        ui.printLine(dvm_meas, 20, 4);
        dvm_set_pulse_count();
        do_toggle();
        wifi_ui_message = dvm_meas + " V" + dvm_get_mode_str();
        trigger_control.triggered = is_triggered = repeat_on.get();
    }
}

void set_dvm_trigger()
{
    dvm_set_pulse_count();
    trigger_control.next_time = millis() + 1;
    trigger_control.triggered = true;
    // Serial.println("Set Stretch Trigger: " + String(spank_engine.delay));
}

void dvm_fxn()
{
    // MyTimer5.begin(30); // beats per sec
    wifi_ui_message = F("Click Activate to start. Refresh to see result.");
    ui.newFxn("Bounce");
    disable_ext_trigger();
    dvm_adjust_param(0, 0); // prints result
    trigger_fxn = dvm_trigger;
}

void dvm_begin()
{
    dvm_mode.begin(false);
    dvm_mode.xfer();
}
