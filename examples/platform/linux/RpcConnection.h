/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <cstdint>

#include <lib/core/CHIPError.h>

// Establishes the pigweed RPC connection to the jf-admin-app RPC server.
// The address and port are resolved by the caller (either from the
// --rpc-server-ip command line argument or via mDNS after the Anchor
// Administrator has been commissioned).
CHIP_ERROR RpcConnect(const char * serverIp, uint16_t serverPort);

// Returns true when the RPC server IPv6 address was supplied manually through
// the --rpc-server-ip command line argument. In that case the connection is
// established at startup and no mDNS discovery is performed.
bool RpcServerAddressProvidedManually();

// Returns the RPC server port to use when establishing the connection. This is
// a fixed pigweed listen port (default) that may be overridden through the
// --rpc-server-port command line argument. It is not discoverable over Matter.
uint16_t RpcServerPort();
