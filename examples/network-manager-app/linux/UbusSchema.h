/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <libubus.h>

#ifdef __cplusplus
extern "C" {
#endif

// Note these constants are defined in a C file because the macros
// used to generate these structures don't compile correctly in C++.

extern const struct ubus_object_type kMatterUbusObjectType;

int MatterUbusHandleStatus(struct ubus_context * ctx, struct ubus_object * obj, struct ubus_request_data * req, const char * method,
                           struct blob_attr * msg);

#ifdef __cplusplus
}
#endif
