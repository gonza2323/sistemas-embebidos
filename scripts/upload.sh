#!/bin/env bash

./compile.sh
arduino-cli upload -p /dev/arduino --fqbn arduino:avr:uno "./arduino/arduino.ino"
