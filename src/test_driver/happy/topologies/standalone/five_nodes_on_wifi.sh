#!/bin/bash

#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2018 Google LLC.
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

# Steps to manually build five_nodes_on_wifi.json

happy-node-add 00-WifiNode-TxFourMulticastAddresses
happy-node-add 01-WifiNode-RxOneMulticastAddress
happy-node-add 02-WifiNode-RxTwoMulticastAddresses
happy-node-add 03-WifiNode-RxThreeMulticastAddresses
happy-node-add 04-WifiNode-RxFourMulticastAddresses

happy-network-add WifiNetwork wifi
happy-network-address WifiNetwork 2001:db8:1:2::
happy-network-address WifiNetwork 192.168.1.0

happy-node-join 00-WifiNode-TxFourMulticastAddresses WifiNetwork
happy-node-join 01-WifiNode-RxOneMulticastAddress WifiNetwork
happy-node-join 02-WifiNode-RxTwoMulticastAddresses WifiNetwork
happy-node-join 03-WifiNode-RxThreeMulticastAddresses WifiNetwork
happy-node-join 04-WifiNode-RxFourMulticastAddresses WifiNetwork

# happy-state -s five_nodes_on_wifi.json
