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

void print_push_trig()
{
    ui.printLine(F("Then Push Trig"), LINE_2, 1);
}

void print_please_wait()
{
    ui.printLine(F("Please wait..."), LINE_1, 1);
}

uint16_t readLevel()
{
    uint32_t readAccumulator = 0;

    for (int i = 0; i < ADC_READS_COUNT; ++i)
        readAccumulator += readADC(ADC_DC);

    uint16_t readValue = readAccumulator >> ADC_READS_SHIFT;

    // if (readValue < (ADC_RANGE >> 1))
    //     readValue += ADC_RANGE;

    Serial.print("ADC Level = ");
    Serial.println(readValue);

    return readValue;
}

void adc_correct_offset()
{
    int meas = 0;

    offsetCorrectionValue.put(0);
    gainCorrectionValue.put(0x600);

    Serial.print(F("\r\nOffset correction (@gain = "));
    Serial.print(gainCorrectionValue.get());
    Serial.println(F(" (min gain))"));

    // Set default correction values and enable correction
    analogReadCorrection(offsetCorrectionValue.get(), gainCorrectionValue.get());
    readLevel(); // settling time

    // for (int offset = 0; offset < (int)(ADC_OFFSETCORR_MASK >> 1); ++offset)
    for (int offset = -100; offset < 100; ++offset)
    {
        analogReadCorrection(offset, gainCorrectionValue.get());
        // readLevel(); // settling time

        Serial.print(F("   Offset = "));
        Serial.print(offset);
        Serial.print(", ");

        meas = readLevel();

        if (meas < 20 && meas == save_value && tries++ > 3)
        {
            break;
        }
        save_value = meas;
        offsetCorrectionValue.put(offset);
    }
}

void adc_correct_gain()
{
    Serial.println(F("\r\nGain correction"));
    print_please_wait();
    uint16_t i;
    for (i = ADC_MIN_GAIN; i < ADC_MAX_GAIN && (abs(mid_cal - 2048) >= 2); i++)
    {
        analogReadCorrection(offsetCorrectionValue.get(), i);
        delay(1); // offset
        Serial.println("Gain: " + String(i));
        mid_cal = readLevel();
    }
    gainCorrectionValue.put(i);
}

void adc_summary()
{
    Serial.println(F("\r\n==================\r\n"));
    Serial.print(F("Low Meas: "));
    Serial.println(String(low_cal));
    Serial.print(F("Mid Meas: "));
    Serial.println(String(mid_cal));
    Serial.print(F("High Meas: "));
    Serial.println(String(hi_cal));

    Serial.println(" ");
    int low_diff = mid_cal - low_cal;
    int hi_diff = hi_cal - mid_cal;
    Serial.print(F("Low Diff: "));
    Serial.println(String(low_diff));
    Serial.print(F("High Diff: "));
    Serial.println(String(hi_diff));
    Serial.print(F("Error: "));
    Serial.println(String(hi_diff - low_diff));
    Serial.println(" ");

    Serial.print(F("Gain Corr: "));
    Serial.println(String(gainCorrectionValue.get()));
    Serial.print(F("Offset Corr: "));
    Serial.println(String(offsetCorrectionValue.get()));
    Serial.println(F("\r\n=================="));
}

void adc_cal()
{
    ui.fill(BLACK, 16);
    ui.t.clrDown("8");
    switch (adc_cal_screen)
    {
    case 0:
        low_cal = 0;
        mid_cal = 0;
        hi_cal = 0;
        save_value = 0;
        tries = 0;
        ui.printLine(F("Plug +5V to Sig In"), LINE_1, 1);
        print_push_trig();
        adc_cal_screen++;
        break;
    case 1:
        print_please_wait();
        adc_correct_offset();
        ui.printLine(F("Plug +0V to Sig In"), LINE_1, 1);
        print_push_trig();
        adc_cal_screen++;
        break;
    case 2:
        adc_correct_gain();
        ui.printLine(F("Plug +4V to Sig In"), LINE_1, 1);
        print_push_trig();
        adc_cal_screen++;
        break;
    case 3:
        print_please_wait();
        delay(1000); // offset
        low_cal = readLevel();
        ui.printLine(F("Plug 0V to Sig In"), LINE_1, 1);
        print_push_trig();
        adc_cal_screen++;
        break;
    case 4:
        print_please_wait();
        delay(1000); // mid
        mid_cal = readLevel();
        ui.printLine(F("Plug -4V to Sig In"), LINE_1, 1);
        print_push_trig();
        ui.printLine("Low Diff: " + String(mid_cal - low_cal), LINE_3, 1);
        adc_cal_screen++;
        break;
    case 5:
        print_please_wait();
        delay(1000); // gain
        hi_cal = readLevel();
        ui.printLine(F("Calibration complete"), LINE_1, 1);
        ui.printLine("Hi Diff: " + String(hi_cal - mid_cal), LINE_3, 1);
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
    // delay(100);
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
        Serial.println(F("*** Setting default reference! ***"));
        analogReference(AR_DEFAULT);
    }
}
