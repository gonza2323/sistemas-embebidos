#!/bin/env bash

./compile.sh
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno arduino/arduino.ino
