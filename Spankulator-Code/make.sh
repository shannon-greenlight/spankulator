#!/bin/zsh

./bin/arduino-cli compile -b arduino:samd:nano_33_iot --output-dir ./out --libraries ./Spankulator/libraries Spankulator

