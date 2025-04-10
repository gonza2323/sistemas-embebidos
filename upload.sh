#!/bin/env bash

tp_dir="$1"

if [ -z "$tp_dir" ] || [ ! -d "./$tp_dir" ]; then
  echo "El uso es: $0 <directorio-tp>"
  exit 1
fi

./compile.sh "$tp_dir"
"$HOME/.local/bin/arduino-cli" upload -p /dev/arduino --fqbn arduino:avr:uno "./$tp_dir/arduino/arduino.ino"
