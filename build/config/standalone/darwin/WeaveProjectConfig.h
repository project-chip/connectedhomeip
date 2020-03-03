/*
 *
 *    Copyright (c) 2017 Nest Labs, Inc.
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
 *      Alternate Weave project configuration for building standalone without OpenSSL.
 *
 */
#ifndef WEAVEPROJECTCONFIG_OSX_H
#define WEAVEPROJECTCONFIG_OSX_H

#include "../WeaveProjectConfig.h"
#define INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT 0

#endif /* WEAVEPROJECTCONFIG_H */
