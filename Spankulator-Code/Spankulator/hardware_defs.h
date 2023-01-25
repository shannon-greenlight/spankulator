// Dude, ensure this library description is only included once
#ifndef Hardware_defs_h
#define Hardware_defs_h

#define DAC_FS 1023

// #define STATUS_ROW "5"
// #define FXN_ROW "6"
// #define DEBUG_ROW "21"
// #define TERMINAL_BASE 8

#define WAVEFORM_PARTS 128
// 30 beats per second
#define HEART_RATE 30

#define ADC_PIN A2

#define ADC_BITS 12
// #define ADC_FS (1 << ADC_BITS) - 1
#define ADC_FS 4095
// #define ADC_MID (1 << ADC_BITS - 1)
#define ADC_MID 2048
#define ADC_SCALE -409.6

#define ADC_READS_SHIFT 8
#define ADC_READS_COUNT (1 << ADC_READS_SHIFT)

#define ADC_MIN_GAIN 0x0400
#define ADC_UNITY_GAIN 0x0800
#define ADC_MAX_GAIN (0x1000 - 1)
#define ADC_RESOLUTION_BITS 12
#define ADC_RANGE 4096
// (1 << ADC_RESOLUTION_BITS)
#define ADC_TOP_VALUE 4095
// (ADC_RANGE - 1)

#define MAX_TOP_VALUE_READS 10

#define DAC_BITS 10
// #define DAC_FS (1 << DAC_BITS)
//  #define DAC_FS 1023
// #define DAC_MID (1 << DAC_BITS - 1)
#define DAC_MID 512

// #define ADC_DAC_SHIFT (ADC_BITS - DAC_BITS)
#define ADC_DAC_SHIFT 2

#endif
