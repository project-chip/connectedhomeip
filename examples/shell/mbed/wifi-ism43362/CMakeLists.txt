# Copyright (c) 2020 ARM Limited. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(wifi-ism43362 INTERFACE)

target_include_directories(wifi-ism43362
    INTERFACE
        .
        ISM43362
        ISM43362/ATParser
        ISM43362/ATParser/BufferedSpi
        ISM43362/ATParser/BufferedSpi/Buffer
)

target_sources(wifi-ism43362
    INTERFACE
        ISM43362Interface.cpp
        ISM43362/ISM43362.cpp
        ISM43362/ATParser/ATParser.cpp
        ISM43362/ATParser/BufferedSpi/BufferedPrint.c
        ISM43362/ATParser/BufferedSpi/BufferedSpi.cpp
        ISM43362/ATParser/BufferedSpi/Buffer/MyBuffer.cpp
)
