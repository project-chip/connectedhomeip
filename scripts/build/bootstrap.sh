#!/bin/bash

if [[ ! -f build/default/config.status ]]; then
  mkdir -p build/default
  (cd build/default &&
    ../../bootstrap-configure -C --enable-debug --enable-coverage)
elif [[ configure.ac -nt configure ]]; then
  ./bootstrap
else
  ./bootstrap -w make
fi
