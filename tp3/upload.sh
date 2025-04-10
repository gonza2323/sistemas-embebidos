#!/bin/env bash

./compile.sh
$HOME/.local/bin/arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno arduino/arduino.ino
