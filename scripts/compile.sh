#!/usr/bin/env bash

arduino-cli compile --fqbn arduino:avr:uno --libraries "./arduino/libraries" --output-dir "./arduino/build" "./arduino/arduino.ino"
