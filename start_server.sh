#!/usr/bin/env bash

tp_dir="$1"
shift

if [ -z "$tp_dir" ] || [ ! -d "./$tp_dir" ]; then
  echo "El uso es: $0 <directorio-tp>"
  exit 1
fi

source ./.venv/bin/activate > /dev/null
flask --app "$tp_dir.website.main" run --host=0.0.0.0 "$@"