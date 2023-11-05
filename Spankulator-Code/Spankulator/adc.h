#include "SAMD_AnalogCorrection.h"

// #define ADC_GND_PIN A1

EEPROM_Int offsetCorrectionValue = EEPROM_Int(-100, 100);
// int offsetCorrectionValue = 0;
//  uint16_t gainCorrectionValue = 0x600;
EEPROM_Int gainCorrectionValue = EEPROM_Int(ADC_MIN_GAIN, ADC_MAX_GAIN);

uint8_t adc_cal_screen = 0;

uint16_t low_cal = 0;
uint16_t mid_cal = 0;
uint16_t hi_cal = 0;
int save_value = 0;
int tries = 0;

void print_result(String s, bool printCR = false, bool add_to_message = false)
{
    if (printCR)
    {
        ui.terminal_println(s);
    }
    else
    {
        ui.terminal_print(s);
    }
    if (add_to_message)
    {
        wifi_ui_message += s;
    }
    // wifi_ui_message += s;
}

void adc_printLine(String s, int line, int size = 1)
{
    ui.printLine(s, line, size);
    wifi_ui_message += "<br>";
    wifi_ui_message += s;
}

void print_push_trig()
{
    adc_printLine(F("Then Push Activate"), LINE_2, 1);
}

void print_please_wait()
{
    adc_printLine(F("Please wait..."), LINE_1, 1);
    send_data_to_USB(' ');
}

uint16_t readLevel()
{
    uint32_t readAccumulator = 0;

    for (int i = 0; i < ADC_READS_COUNT; ++i)
        readAccumulator += analogRead(ADC_DC);

    uint16_t readValue = readAccumulator >> ADC_READS_SHIFT;

    // if (readValue < (ADC_RANGE >> 1))
    //     readValue += ADC_RANGE;
    // uint16_t readValue = readADC(ADC_CAL);

    ui.terminal_debug("ADC Level = " + String(readValue));
    // print_result(readValue,true);

    return readValue;
}

void adc_correct_offset()
{
    int meas = 0;

    offsetCorrectionValue.put(0);
    gainCorrectionValue.put(0x600);

    // print_result(F("\r\nOffset correction (@gain = "));
    // print_result(gainCorrectionValue.get());
    // print_result(F(" (min gain))"),true);

    // Set default correction values and enable correction
    analogReadCorrection(offsetCorrectionValue.get(), gainCorrectionValue.get());
    readLevel(); // settling time

    // for (int offset = 0; offset < (int)(ADC_OFFSETCORR_MASK >> 1); ++offset)
    for (int offset = -100; offset < 100; ++offset)
    {
        analogReadCorrection(offset, gainCorrectionValue.get());
        offsetCorrectionValue.put(offset);
        // readLevel(); // settling time

        print_result(F("   Offset = "));
        print_result(String(offset), true);
        // print_result(", ");

        meas = readLevel();

        if (meas < 20 && meas == save_value && tries++ > 3)
        {
            break;
        }
        save_value = meas;
    }
    ui.terminal_clrDown("8");
}

void adc_correct_gain()
{
    // print_result(F("\r\nGain correction"),true);
    print_please_wait();
    mid_cal = readLevel();
    uint16_t i;
    for (i = ADC_MIN_GAIN; i < ADC_MAX_GAIN && (abs(mid_cal - 2048) >= 2); i++)
    {
        analogReadCorrection(offsetCorrectionValue.get(), i);
        delay(1); // offset
        print_result("Gain: " + String(i), true);
        mid_cal = readLevel();
    }
    gainCorrectionValue.put(i);
    ui.terminal_clrDown("8");
}

void adc_summary()
{
    // wifi_ui_message = "";
    print_result(F("\r\n==================\r\n"));
    print_result(F("Low Meas: "));
    print_result(String(low_cal), true);
    print_result(F("Mid Meas: "));
    print_result(String(mid_cal), true);
    print_result(F("High Meas: "));
    print_result(String(hi_cal), true);

    print_result(" ", true);
    int low_diff = mid_cal - low_cal;
    int hi_diff = hi_cal - mid_cal;
    print_result(F("Low Diff: "));
    print_result(String(low_diff), true);
    print_result(F("High Diff: "));
    print_result(String(hi_diff), true);
    print_result(F("Error: "));
    print_result(String(hi_diff - low_diff), true);
    print_result(" ", true);

    print_result(F("Gain Corr: "));
    print_result(String(gainCorrectionValue.get()), true);
    print_result(F("Offset Corr: "));
    print_result(String(offsetCorrectionValue.get()), true);
    print_result(F("\r\n=================="), true);
}

void adc_cal()
{
    is_triggered = false; // make sure next trig toggles to true
    ui.fill(BLACK, 16);
    ui.terminal_clrDown("8");
    wifi_ui_message = "";
    switch (adc_cal_screen)
    {
    case 0:
        low_cal = 0;
        mid_cal = 0;
        hi_cal = 0;
        save_value = 0;
        tries = 0;
        adc_printLine(F("Plug +5V to Sig In"), LINE_1, 1);
        print_push_trig();
        adc_cal_screen++;
        break;
    case 1:
        print_please_wait();
        adc_correct_offset();
        adc_printLine(F("Plug +0V to Sig In"), LINE_1, 1);
        print_push_trig();
        adc_cal_screen++;
        break;
    case 2:
        adc_correct_gain();
        adc_printLine(F("Plug +4V to Sig In"), LINE_1, 1);
        print_push_trig();
        adc_cal_screen++;
        break;
    case 3:
        print_please_wait();
        delay(1000); // offset
        low_cal = readLevel();
        adc_printLine(F("Plug 0V to Sig In"), LINE_1, 1);
        print_push_trig();
        adc_cal_screen++;
        break;
    case 4:
        print_please_wait();
        delay(1000); // mid
        mid_cal = readLevel();
        adc_printLine(F("Plug -4V to Sig In"), LINE_1, 1);
        print_push_trig();
        // adc_printLine("Low Diff: " + String(mid_cal - low_cal), LINE_3, 1);
        adc_cal_screen++;
        break;
    case 5:
        print_please_wait();
        delay(1000); // gain
        hi_cal = readLevel();
        adc_printLine(F("Calibration complete"), LINE_1, 1);
        // adc_printLine("Hi Diff: " + String(hi_cal - mid_cal), LINE_3, 1);
        adc_summary();
        adc_cal_screen = 0;
        break;

    default:
        break;
    }
}

float adc_scale_meas(int meas, byte mode)
{
    if (mode)
    {
        return meas / ADC_SCALE;
    }
    else
    {
        return pow(meas / ADC_SCALE, 2.0);
    }
}

void adc_begin()
{
    offsetCorrectionValue.begin(false);
    offsetCorrectionValue.xfer();
    gainCorrectionValue.begin(false);
    gainCorrectionValue.xfer();
    // delay(3000);
    // print_result("ADC Begin offset: " + String(offsetCorrectionValue.get()) + " gain: " + String(gainCorrectionValue.get()),true);
    // offsetCorrectionValue.put(0);
    // gainCorrectionValue.put(2048);
    analogReadCorrection(offsetCorrectionValue.get(), gainCorrectionValue.get());
}

void adc_init()
{
    offsetCorrectionValue.put(0);
    gainCorrectionValue.put(ADC_UNITY_GAIN);
}

void adc_config_hardware()
{
    // DAC->CTRLB.bit.REFSEL = 0;
    ADC->SAMPCTRL.reg = 0x00; // sets 200usec sample rate
    analogReadResolution(ADC_BITS);
    analogReference(AR_EXTERNAL);
    int raw = analogRead(A6);
    // in case there is no ext ref
    if (raw > 1000)
    {
        print_result(F("*** Setting default reference! ***"), true);
        analogReference(AR_DEFAULT);
    }
}
