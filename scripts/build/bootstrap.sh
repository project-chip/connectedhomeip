#!/bin/bash

# if [[ ! -f build/default/config.status ]]; then mkdir -p build/default; (cd build/default && ../../bootstrap-configure --enable-debug --enable-coverage); else ./bootstrap -w make; fi

if [[ ! -f build/default/config.status ]]; then
    mkdir -p build/default
    (cd build/default &&
        ../../bootstrap-configure --enable-debug --enable-coverage)
elif [[ configure.ac -nt configure ]]; then
    ./bootstrap
else
    ./bootstrap -w make
fi
