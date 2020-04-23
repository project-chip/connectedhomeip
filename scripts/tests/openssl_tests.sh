#!/bin/bash

make -C build/default/src/crypto/ && make -C build/default/src/crypto/tests/ check
