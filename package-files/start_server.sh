#!/usr/bin/env bash

source ./.venv/bin/activate > /dev/null
flask --app ./website/main.py run --host=0.0.0.0 "$@"
