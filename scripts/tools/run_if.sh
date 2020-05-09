#!/bin/bash

if [[ $1 == *"$2"* ]]; then
  "$("$3")"
fi
