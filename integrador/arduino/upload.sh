#!/bin/env bash

arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno arduino.ino
