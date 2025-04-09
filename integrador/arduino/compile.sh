#!/usr/bin/env bash

arduino-cli compile --fqbn arduino:avr:uno --libraries ./libraries --output-dir build
