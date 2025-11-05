/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "Globals.h"
#include "DeviceCallbacks.h"
#include <app/TimerDelegateDefault.h>

LEDWidget statusLED1;
LEDWidget statusLED2;
BluetoothWidget bluetoothLED;
WiFiWidget wifiLED;
WiFiWidget pairingWindowLED;

using namespace chip::app;
using namespace chip::app::Clusters;

DefaultTimerDelegate sTimerDelegate;
IdentifyDelegateImpl sIdentifyDelegate;

RegisteredServerCluster<IdentifyCluster> gIdentifyCluster1(IdentifyCluster::Config(1, sTimerDelegate)
                                                               .WithIdentifyType(Identify::IdentifyTypeEnum::kVisibleIndicator)
                                                               .WithDelegate(&sIdentifyDelegate));

RegisteredServerCluster<IdentifyCluster> gIdentifyCluster2(IdentifyCluster::Config(2, sTimerDelegate)
                                                               .WithIdentifyType(Identify::IdentifyTypeEnum::kVisibleIndicator)
                                                               .WithDelegate(&sIdentifyDelegate));

RegisteredServerCluster<IdentifyCluster> gIdentifyCluster3(IdentifyCluster::Config(3, sTimerDelegate)
                                                               .WithIdentifyType(Identify::IdentifyTypeEnum::kVisibleIndicator)
                                                               .WithDelegate(&sIdentifyDelegate));

RegisteredServerCluster<IdentifyCluster> gIdentifyCluster4(IdentifyCluster::Config(4, sTimerDelegate)
                                                               .WithIdentifyType(Identify::IdentifyTypeEnum::kVisibleIndicator)
                                                               .WithDelegate(&sIdentifyDelegate));
