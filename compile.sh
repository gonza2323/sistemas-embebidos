#!/usr/bin/env bash

tp_dir="$1"

if [ -z "$tp_dir" ] || [ ! -d "./$tp_dir" ]; then
  echo "El uso es: $0 <directorio-tp>"
  exit 1
fi

"$HOME/.local/bin/arduino-cli" compile --fqbn arduino:avr:uno --libraries "./$tp_dir/arduino/libraries" --output-dir "./$tp_dir/arduino/build" "./$tp_dir/arduino/arduino.ino"
