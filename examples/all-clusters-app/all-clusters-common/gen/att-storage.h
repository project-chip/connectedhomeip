
/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

// Prevent multiple inclusion
#pragma once

// Attribute masks modify how attributes are used by the framework
//
// Attribute that has this mask is NOT read-only
#define ATTRIBUTE_MASK_WRITABLE (0x01)
// Attribute that has this mask is saved to a token
#define ATTRIBUTE_MASK_TOKENIZE (0x02)
// Attribute that has this mask has a min/max values
#define ATTRIBUTE_MASK_MIN_MAX (0x04)
// Manufacturer specific attribute
#define ATTRIBUTE_MASK_MANUFACTURER_SPECIFIC (0x08)
// Attribute deferred to external storage
#define ATTRIBUTE_MASK_EXTERNAL_STORAGE (0x10)
// Attribute is singleton
#define ATTRIBUTE_MASK_SINGLETON (0x20)
// Attribute is a client attribute
#define ATTRIBUTE_MASK_CLIENT (0x40)

// Cluster masks modify how clusters are used by the framework
//
// Does this cluster have init function?
#define CLUSTER_MASK_INIT_FUNCTION (0x01)
// Does this cluster have attribute changed function?
#define CLUSTER_MASK_ATTRIBUTE_CHANGED_FUNCTION (0x02)
// Does this cluster have default response function?
#define CLUSTER_MASK_DEFAULT_RESPONSE_FUNCTION (0x04)
// Does this cluster have message sent function?
#define CLUSTER_MASK_MESSAGE_SENT_FUNCTION (0x08)
// Does this cluster have manufacturer specific attribute changed function?
#define CLUSTER_MASK_MANUFACTURER_SPECIFIC_ATTRIBUTE_CHANGED_FUNCTION (0x10)
// Does this cluster have pre-attribute changed function?
#define CLUSTER_MASK_PRE_ATTRIBUTE_CHANGED_FUNCTION (0x20)
// Cluster is a server
#define CLUSTER_MASK_SERVER (0x40)
// Cluster is a client
#define CLUSTER_MASK_CLIENT (0x80)

// Command masks modify meanings of commands
//
// Is sending of this client command supported
#define COMMAND_MASK_OUTGOING_CLIENT (0x01)
// Is sending of this server command supported
#define COMMAND_MASK_OUTGOING_SERVER (0x02)
// Is receiving of this client command supported
#define COMMAND_MASK_INCOMING_CLIENT (0x04)
// Is receiving of this server command supported
#define COMMAND_MASK_INCOMING_SERVER (0x08)
// Is this command manufacturer specific?
#define COMMAND_MASK_MANUFACTURER_SPECIFIC (0x10)
