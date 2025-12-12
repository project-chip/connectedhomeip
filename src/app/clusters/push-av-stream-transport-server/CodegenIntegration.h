/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include "push-av-stream-transport-delegate.h"
#include <app/clusters/tls-certificate-management-server/tls-certificate-management-server.h>
#include <app/clusters/tls-client-management-server/TlsClientManagementCluster.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

/// Sets the given delegate on an endpoint configured via code-generation
void SetDelegate(chip::EndpointId endpointId, PushAvStreamTransportDelegate * delegate);

/// Sets the given TLS Client Management delegate on an endpoint configured via code-generation
void SetTLSClientManagementDelegate(chip::EndpointId endpointId, TlsClientManagementDelegate * delegate);

/// Sets the given TLS Certificate Management delegate on an endpoint configured via code-generation
void SetTlsCertificateManagementDelegate(chip::EndpointId endpointId, TlsCertificateManagementDelegate * delegate);

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
