#!/bin/bash

"$config_options" = "$@"

if [[ ! -f build/default/config.status ]]; then
  mkdir -p build/default
  (cd build/default &&
    ../../bootstrap-configure --enable-debug --enable-coverage "$config_options")
elif [[ configure.ac -nt configure ]]; then
  ./bootstrap
else
  ./bootstrap -w make
fi
