/*
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          Header for the CHIP Manager class.
 *
 *          This class provides various facilities through CHIP's inner
 *          implementation. It should be removed when a more accessible
 *          CHIP api is ready.
 *
 */

#ifndef CHIP_MANAGER_H
#define CHIP_MANAGER_H

#include <new>

#include <core/CHIPError.h>
#include <inet/InetLayer.h>
#include <inet/UDPEndPoint.h>
#include <support/CHIPLogging.h>
#include <system/SystemLayer.h>

using namespace chip;

class ChipManager
{
public:
    // Initialize the System Layer and Init Layer
    CHIP_ERROR InitLayers();
    System::Layer * SystemLayer();
    Inet::InetLayer * InetLayer();
    void ServiceEvents();

    ChipManager() : mSystem(new System::Layer()), mInet(new Inet::InetLayer()) {}

private:
    System::Layer * mSystem;
    Inet::InetLayer * mInet;
};

#endif // CHIP_MANAGER_H
