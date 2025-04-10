#!/usr/bin/env bash

$HOME/.local/bin/arduino-cli compile --fqbn arduino:avr:uno --libraries ./arduino/libraries --output-dir ./arduino/build ./arduino/arduino.ino
