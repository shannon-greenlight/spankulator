
uint8_t dac_cal_screen = 0;

void dac_correct_offset()
{
    Serial.println(F("\r\nOffset correction"));
}

void dac_correct_gain()
{
    Serial.println(F("\r\nGain correction"));
}

void dac_cal()
{
    is_triggered = false; // make sure next trig toggles to true
    // A change of 8 on the DAC should result in a change of 1/12V on CV output
    String line1;
    String line2;
    ui.fill(BLACK, 16);
    ui.t.clrDown("8");
    switch (dac_cal_screen)
    {
    case 0:
        line1 = F("DAC Zero: Adj OFFSET");
        line2 = F("for CV=0.0V");
        write_dac(511); // adjust for 0V

        dac_cal_screen++;
        break;
    case 1:
        line1 = F("DAC +Mid: Adj GAIN");
        line2 = F("for CV=2.0V");
        write_dac(DAC_MID - 193); // adjust for 2V
        dac_cal_screen++;
        break;
    case 2:
        line1 = F("DAC -Mid: Adj GAIN");
        line2 = F("for CV=-2.0V");
        write_dac(DAC_MID + 192); // adjust for -2V
        dac_cal_screen++;
        break;
    case 3:
        line1 = F("Calibration complete");
        line2 = F("Push Activate to redo");
        // dac_summary();
        dac_cal_screen = 0;
        break;

    default:
        break;
    }
    ui.printLine(line1, LINE_1, 1);
    ui.printLine(line2, LINE_2, 1);
    wifi_ui_message = line1 + "<br>" + line2;
}
