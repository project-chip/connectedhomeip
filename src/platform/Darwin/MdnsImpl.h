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

#ifndef CHIP_PLATFORM_DARWIN_MDNS_IMPL_H_
#define CHIP_PLATFORM_DARWIN_MDNS_IMPL_H_

#include <sys/select.h>
#include <unistd.h>

#include "platform/Mdns.h"

namespace chip {
namespace DeviceLayer {

void UpdateMdnsDataset(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet, int & maxFd, timeval & timeout);

void ProcessMdns(fd_set & readFdSet, fd_set & writeFdSet, fd_set & errorFdSet);

} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_PLATFORM_DARWIN_MDNS_IMPL_H_
